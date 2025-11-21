# Minimal Dockerfile (not optimized)
FROM rust:1.81 as builder
WORKDIR /app
COPY . .
RUN cargo build --release

FROM debian:stable-slim
WORKDIR /app
COPY --from=builder /app/target/release/slice-orchestrator /usr/local/bin/slice-orchestrator
EXPOSE 8080
ENV RUST_LOG=info
CMD ["slice-orchestrator"]

