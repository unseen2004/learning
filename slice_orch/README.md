# Slice Orchestrator

A Rust control plane prototype for 5G network slice lifecycle management. Implements:
- REST health & ping endpoints (Axum)
- gRPC services (Slice, Telemetry, Policy) generated from protobufs via build.rs (tonic)
- Basic allocator with capacity enforcement & metrics
- Optional Kafka telemetry ingestion (feature `kafka`)
- Simplistic policy verification & DSL parsing stubs
- Constraint feasibility + optimization skeleton
- Optional OpenTelemetry tracing + metrics (feature `otel`) with OTLP exporter

## Run

```bash
cargo run
```

Visit http://localhost:8080/health

### Enable Kafka + OTEL
```bash
cargo run --no-default-features --features "rest grpc otel kafka" -- \
  ORCH__KAFKA_BROKERS=localhost:9092 \
  ORCH__OTLP_ENDPOINT=http://localhost:4317
```

## Configuration
Environment variables with prefix `ORCH__` override defaults (e.g. `ORCH__REST_PORT=9090`). Key vars:
- `ORCH__GRPC_PORT` (default 50051)
- `ORCH__TOTAL_BANDWIDTH_CAPACITY_MBPS` (default 10000)
- `ORCH__KAFKA_BROKERS` (enables consumer if set) 
- `ORCH__KAFKA_TELEMETRY_TOPIC` (default telemetry.samples)
- `ORCH__OTLP_ENDPOINT` (enables OTLP exporter if set)

## Development
Protos in `proto/` are (re)generated automatically on build (`build.rs`). Generated code is emitted into OUT_DIR and included at compile time.


## Testing
```bash
cargo test
```

## License
Apache-2.0
