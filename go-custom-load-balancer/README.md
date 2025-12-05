# Go Custom HTTP Load Balancer

A production-quality HTTP load balancer built in Go that distributes traffic across multiple backend servers using configurable algorithms, health monitoring, and automatic failover.

## Features

- **Load Balancing Algorithms**
  - **Round Robin**: Distributes requests sequentially across all backends
  - **Least Connections**: Routes to the backend with fewest active connections

- **Health Checks**: Automatically monitors backend health and removes dead servers from the pool

- **Automatic Retries**: Failed requests are automatically retried on the next available backend

- **Concurrency Safe**: Uses `sync.RWMutex` and `sync/atomic` for thread-safe operations

- **HTTP Headers**: Properly sets `X-Forwarded-For` and `X-Forwarded-Host` headers

## Installation

```bash
go build -o loadbalancer .
```

## Usage

### Start Backend Servers (Demo)

```bash
# Terminal 1
go run demo/backend_server.go -port 9001

# Terminal 2  
go run demo/backend_server.go -port 9002

# Terminal 3
go run demo/backend_server.go -port 9003
```

### Start Load Balancer

```bash
# Round Robin (default)
go run main.go -port 8080 \
  -backends "http://localhost:9001,http://localhost:9002,http://localhost:9003" \
  -algorithm roundrobin

# Least Connections
go run main.go -port 8080 \
  -backends "http://localhost:9001,http://localhost:9002,http://localhost:9003" \
  -algorithm leastconn
```

### Test It

```bash
# Send requests
curl http://localhost:8080

# Watch distribution (6 requests = 2 per backend with round robin)
for i in {1..6}; do curl -s http://localhost:8080 | head -1; done
```

## Command Line Flags

| Flag | Default | Description |
|------|---------|-------------|
| `-port` | 8080 | Load balancer listening port |
| `-backends` | (required) | Comma-separated backend URLs |
| `-algorithm` | roundrobin | Algorithm: `roundrobin` or `leastconn` |
| `-health-interval` | 10s | Health check interval |
| `-health-timeout` | 2s | Health check timeout |
| `-max-retries` | 3 | Max retry attempts for failed requests |

## Project Structure

```
├── main.go              # Entry point with CLI flags
├── backend/
│   └── backend.go       # Backend server representation
├── pool/
│   └── pool.go          # Thread-safe server pool
├── balancer/
│   ├── balancer.go      # Algorithm interface
│   ├── roundrobin.go    # Round Robin implementation
│   └── leastconn.go     # Least Connections implementation
├── proxy/
│   └── proxy.go         # Reverse proxy with retry logic
├── health/
│   └── checker.go       # Background health monitoring
└── demo/
    └── backend_server.go # Test backend server
```

## Testing

```bash
# Run all tests with race detection
go test ./... -v -race
```

## How It Works

1. **Request arrives** at the load balancer
2. **Algorithm selects** the next backend (Round Robin or Least Connections)
3. **Request is proxied** to the selected backend
4. **On failure**: Backend is marked dead, request retries on next backend
5. **Health checker** periodically pings backends and updates their status
6. **Recovered backends** are automatically added back to the pool

## License

MIT
