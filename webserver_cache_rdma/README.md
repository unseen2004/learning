# C++17 Multithreaded Static Web Server with LRU Cache and Optional RDMA Fast Path

A high-performance HTTP/1.1 static file server written in modern C++ with:
- Asynchronous networking (Boost.Asio)
- Keep-alive and request pipelining
- In-memory LRU cache (ETag + Last-Modified metadata)
- Optional RDMA fast path using rdma-core (SEND/RECV) for trusted clients
- Docker packaging

This server targets low-latency repeat reads of static assets and provides an RDMA endpoint for internal low-overhead data delivery.

## Features

- HTTP/1.1
  - GET and HEAD
  - Keep-Alive
  - Request pipelining (multiple requests queued and answered in order)
  - MIME type detection
  - Path traversal protection
- Caching
  - Thread-safe in-memory LRU cache with size cap
  - ETag and Last-Modified support metadata
- RDMA (optional)
  - rdma_cm + ibverbs
  - Pre-posted RECVs per connection
  - Custom binary protocol over SEND/RECV (GET, PING)
  - Shared cache with HTTP path
- Operational
  - Clean shutdown on SIGINT/SIGTERM
  - Simple metrics endpoint (/metrics)
  - Docker images for build and runtime

## Project Structure

```
.
├── CMakeLists.txt               # Build configuration (C++17, fmt, Boost.Asio, optional RDMA)
├── Dockerfile                   # Multi-stage build with optional RDMA runtime
├── public/                      # Default document root (example content)
├── src/
│   ├── main.cpp                 # Entry point; parses flags; starts HTTP and optional RDMA
│   ├── server.{hpp,cpp}         # HTTP acceptor; creates Session per connection
│   ├── session.{hpp,cpp}        # Per-connection HTTP state; parsing, responses, pipelining
│   ├── signals.{hpp,cpp}        # Graceful shutdown via signals
│   ├── util/
│   │   ├── config.{hpp,cpp}     # CLI flags parsing and config
│   │   ├── logging.{hpp,cpp}    # Logging helpers (fmt)
│   │   ├── metrics.{hpp,cpp}    # Simple counters and /metrics formatter
│   │   └── time.{hpp,cpp}       # HTTP date helpers
│   ├── http/
│   │   ├── parser.{hpp,cpp}     # Minimal HTTP/1.1 parser (request line + headers)
│   │   ├── request.{hpp,cpp}    # Request model + helpers
│   │   ├── response.hpp         # Response builder + serializer
│   │   ├── headers.hpp          # Header casing helpers
│   │   └── mime.{hpp,cpp}       # File extension → Content-Type
│   ├── fs/
│   │   ├── path_utils.{hpp,cpp} # URL → filesystem path, traversal guard
│   │   └── file_reader.{hpp,cpp}# Read files + metadata for caching
│   ├── cache/
│   │   ├── lru_cache.{hpp,cpp}  # Thread-safe in-memory LRU cache
│   └── rdma/                    # Optional RDMA fast path
│       ├── rdma_server.{hpp,cpp}# CM + CQ setup, pollers, connection lifecycle
│       ├── connection.{hpp,cpp} # Per-connection state; SEND/RECV flow; cache integration
│       └── protocol.{hpp,cpp}   # Binary protocol definitions and helpers
└── docs/
    └── USAGE.md                 # Optional detailed usage (README summarizes below)
```

## Quick Start

Prerequisites:
- CMake 3.16+, Git
- A C++17 compiler (GCC 10+/Clang 12+/MSVC 2019+)
- Boost.System (headers, libs)
- RDMA (optional): rdma-core (librdmacm, libibverbs) on Linux with RDMA HW or soft-RoCE

Build (Release):
```
mkdir -p build
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build -j
```

Run HTTP server:
```
./build/webserver --port 8080 --threads 4 --doc-root ./public --cache.mem-mb 128
```

Docker:
```
docker build -t webserver:latest .
docker run --rm -p 8080:8080 -v $(pwd)/public:/site webserver:latest \
  /app/webserver --port 8080 --doc-root /site --threads 0 --cache.mem-mb 128
```

## Configuration

HTTP flags:
- --port N: HTTP port (default 8080)
- --threads N: number of worker threads (0 = hardware concurrency)
- --doc-root PATH: directory to serve (default ./public)
- --cache.mem-mb N: in-memory cache capacity (default 128)
- --read-timeout-ms N: per-read timeout (default 5000)
- --write-timeout-ms N: per-write timeout (default 5000)
- --keepalive-timeout-ms N: idle keep-alive timeout (default 10000)
- --max-request-line N: max request line bytes (default 8192)
- --max-header-bytes N: total header bytes cap (default 32768)

RDMA flags (effective when compiled with ENABLE_RDMA=ON):
- --rdma.enable
- --rdma.bind IP (default 0.0.0.0)
- --rdma.port N (default 7471)
- --rdma.pollers N (default 1)
- --rdma.recv-bufs N (default 64)
- --rdma.recv-size N (default 4096)
- --rdma.send-chunk N (default 32768)
- --rdma.max-sends N (default 64)

Enable RDMA at build time:
```
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release -DENABLE_RDMA=ON
cmake --build build -j
```

Run with RDMA:
```
./build/webserver --port 8080 --threads 4 --doc-root ./public --cache.mem-mb 128 \
  --rdma.enable --rdma.bind 0.0.0.0 --rdma.port 7471 --rdma.pollers 1
```

Docker with RDMA:
```
docker build -t webserver:rdma .
# Depending on your host RDMA device and container runtime setup, you may need --net=host and device passthrough.
docker run --rm -p 8080:8080 -p 7471:7471 webserver:rdma \
  /app/webserver --port 8080 --doc-root /site --rdma.enable
```

## RDMA Fast Path Protocol

Binary protocol (little-endian) over SEND/RECV:
- Request
  - struct ReqHeader { uint8 op; uint16 path_len; }
  - op=1 (GET): followed by path bytes (e.g., "/index.html")
  - op=2 (PING): no payload
- Response
  - struct RespHeader { uint16 status; uint64 content_len; uint32 chunk_size; }
  - For GET: server sends RespHeader then sends file payload in chunk_size chunks (each as SEND)
  - For PING: server sends RespHeader with status=200, content_len=0

Client notes:
- Post enough RECVs to receive one header plus all payload chunks:
  - RECVs >= 1 + ceil(content_len / chunk_size)
- Choose chunk_size and RECV buffer count to avoid RNR NACKs under load.

## Pipelining

The HTTP session parses as many full requests as available from the read buffer (without waiting for responses) and enqueues them. Responses are serialized and written strictly in order. If a request includes "Connection: close", the server completes that response and closes the connection.

## Metrics

Text endpoint at /metrics (Prometheus-friendly):
- Counters for requests, response classes, cache hits/misses, bytes served
- RDMA counters: requests, ok/err, bytes

Example:
```
curl -s http://localhost:8080/metrics
```

## Security Notes

- Static serving only; no directory listings
- Path traversal is blocked (canonicalization + root containment checks)
- RDMA endpoint intended for trusted/internal networks only; no authentication built-in
- Consider network ACLs, mTLS at a proxy layer, or deploy RDMA on isolated segments

## Performance Tips

- Increase --threads for multi-core workloads
- Size the in-memory cache (--cache.mem-mb) to hold hot assets
- Tune timeouts for your clients and network
- For RDMA:
  - Increase recv buffers (--rdma.recv-bufs) and chunk size (--rdma.send-chunk) for large payloads
  - Ensure client posts sufficient RECVs to match server’s chunking

## Troubleshooting

- Port already in use
  - Change --port or stop the conflicting service
- RDMA build or runtime errors
  - Ensure librdmacm and libibverbs are installed and devices are available (or soft-RoCE configured)
  - Build with -DENABLE_RDMA=ON
- 404 Not Found
  - Verify --doc-root and that the requested file exists
- Duplicate include in logging.cpp
  - If you see a build warning/error in src/util/logging.cpp due to double include, make sure the file contains a single include:
    ```
    #include "logging.hpp"
    // Inline utilities; no implementation needed.
    ```


## License

Choose a license appropriate for your project (e.g., MIT, Apache-2.0). Add a LICENSE file at the repository root.

## Acknowledgements

- Boost.Asio (networking)
- fmt (formatting)
- rdma-core (librdmacm, libibverbs)