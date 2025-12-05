# Slice Orchestrator

Rust control plane prototype for 5G network slice lifecycle management.

---

## Overview

A 5G network slice orchestrator implementing REST and gRPC services for managing network slice lifecycles. Features resource allocation, telemetry ingestion, policy verification, and observability.

---

## Features

- REST API with health and ping endpoints (Axum)
- gRPC services: Slice, Telemetry, Policy (Tonic)
- Resource allocator with capacity enforcement
- Kafka telemetry ingestion (optional)
- Policy verification and DSL parsing
- Constraint feasibility and optimization
- OpenTelemetry tracing and metrics (optional)

---

## Building

```bash
cargo build --release
```

---

## Running

Basic run:
```bash
cargo run
```

Visit http://localhost:8080/health

With Kafka and OpenTelemetry:
```bash
cargo run --no-default-features --features "rest grpc otel kafka" -- \
  ORCH__KAFKA_BROKERS=localhost:9092 \
  ORCH__OTLP_ENDPOINT=http://localhost:4317
```

---

## Configuration

Environment variables with `ORCH__` prefix override defaults:

- `ORCH__REST_PORT` - REST API port (default 8080)
- `ORCH__GRPC_PORT` - gRPC port (default 50051)
- `ORCH__TOTAL_BANDWIDTH_CAPACITY_MBPS` - Total bandwidth (default 10000)
- `ORCH__KAFKA_BROKERS` - Kafka brokers (enables consumer)
- `ORCH__KAFKA_TELEMETRY_TOPIC` - Kafka topic (default telemetry.samples)
- `ORCH__OTLP_ENDPOINT` - OTLP exporter endpoint

---

## Testing

```bash
cargo test
```

---

## Development

Protocol buffers in `proto/` are automatically generated during build via `build.rs`. Generated code is emitted to OUT_DIR and included at compile time.

---

## License

Apache-2.0

<!-- REPOS-CLI:START -->
[![Rust](https://img.shields.io/badge/rust-%23000000.svg?style=for-the-badge&logo=rust&logoColor=white)](https://www.rust-lang.org/)

**Rust Dependencies:** anyhow, async-trait, axum, config, dashmap, futures, once_cell, opentelemetry, opentelemetry-otlp, opentelemetry_sdk, and more...

## Docker

Build and run with Docker:

```bash
docker build -t slice_orch .
docker run -it slice_orch
```

## Recent Updates

- readme (d6c2581)
- readme (5246513)
- Update README.md (54ebf28)
- feat: readme (24095d9)
- batman (f2e39d0)


<!-- REPOS-CLI:END -->
