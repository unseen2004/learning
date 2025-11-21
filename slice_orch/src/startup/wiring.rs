use crate::config::AppConfig;
use axum::{Router, routing::get, Json};
use serde::Serialize;
use tracing::info;

#[derive(Serialize)]
struct Health { status: &'static str }

fn build_router() -> Router { Router::new().route("/health", get(|| async { Json(Health { status: "ok" }) })) }

pub async fn run_orchestrator(cfg: AppConfig) -> anyhow::Result<impl std::future::Future<Output = ()>> {
    // init tracing (otel optional)
    let _ = crate::telemetry::tracing::init_tracing(&cfg);

    // init allocator capacity
    crate::controller::allocator::init(cfg.total_bandwidth_capacity_mbps);

    // HTTP REST server
    let app = build_router();
    let rest_addr = cfg.rest_socket_addr();
    let rest_listener = tokio::net::TcpListener::bind(rest_addr).await?;
    info!(%rest_addr, "HTTP server listening");
    tokio::spawn(async move { if let Err(e) = axum::serve(rest_listener, app).await { tracing::error!(error=%e, "rest server error"); } });

    // gRPC server
    #[allow(unused)]
    let grpc_handle = {
        let addr = cfg.grpc_socket_addr();
        match crate::api::grpc::spawn_grpc(addr).await { Ok(h) => Some(h), Err(e) => { tracing::warn!(error=%e, "grpc not started"); None } }
    };

    // Kafka consumer (telemetry)
    #[allow(unused)]
    let kafka_handle = if cfg.kafka_enabled() {
        match crate::events::kafka::spawn_kafka_consumer(cfg.kafka_brokers.as_ref().unwrap(), &cfg.kafka_telemetry_topic).await { Ok(h) => Some(h), Err(e) => { tracing::warn!(error=%e, "kafka consumer not started"); None } }
    } else { None };

    Ok(async move { let _ = tokio::signal::ctrl_c().await; drop(grpc_handle); drop(kafka_handle); })
}
