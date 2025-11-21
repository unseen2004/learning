use serde::Deserialize;
use std::net::{SocketAddr, IpAddr, Ipv4Addr};

#[derive(Debug, Clone, Deserialize)]
pub struct AppConfig {
    #[serde(default = "default_host")] pub rest_host: String,
    #[serde(default = "default_port")] pub rest_port: u16,
    // gRPC configuration
    #[serde(default = "default_host")] pub grpc_host: String,
    #[serde(default = "default_grpc_port")] pub grpc_port: u16,
    // Resource capacity (total bandwidth mbps available cluster-wide)
    #[serde(default = "default_capacity")] pub total_bandwidth_capacity_mbps: u32,
    // Kafka telemetry ingestion
    #[serde(default)] pub kafka_brokers: Option<String>,
    #[serde(default = "default_kafka_topic")] pub kafka_telemetry_topic: String,
    // OTLP exporter endpoint (if empty, OTEL disabled even if feature on)
    #[serde(default)] pub otlp_endpoint: Option<String>,
}

fn default_host() -> String { "0.0.0.0".into() }
fn default_port() -> u16 { 8080 }
fn default_grpc_port() -> u16 { 50051 }
fn default_capacity() -> u32 { 10_000 } // 10 Gbps placeholder
fn default_kafka_topic() -> String { "telemetry.samples".into() }

impl AppConfig {
    pub fn load(path: Option<&str>) -> anyhow::Result<Self> {
        crate::config::loader::load(path)
    }
    pub fn rest_socket_addr(&self) -> SocketAddr {
        SocketAddr::new(self.rest_host.parse::<IpAddr>().unwrap_or(IpAddr::V4(Ipv4Addr::UNSPECIFIED)), self.rest_port)
    }
    pub fn grpc_socket_addr(&self) -> SocketAddr {
        SocketAddr::new(self.grpc_host.parse::<IpAddr>().unwrap_or(IpAddr::V4(Ipv4Addr::UNSPECIFIED)), self.grpc_port)
    }
    pub fn kafka_enabled(&self) -> bool { self.kafka_brokers.is_some() }
    pub fn otel_enabled(&self) -> bool { self.otlp_endpoint.is_some() }
}
