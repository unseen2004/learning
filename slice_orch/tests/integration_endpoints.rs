use slice_orchestrator::{startup::run_orchestrator, config::AppConfig};
use slice_orchestrator::api::grpc::pb::sliceorchestrator::{
    slice::v1::slice_service_client::SliceServiceClient,
    slice::v1::SliceSpec as GrpcSliceSpec,
    policy::v1::policy_service_client::PolicyServiceClient,
    policy::v1::Policy,
    telemetry::v1::telemetry_ingest_client::TelemetryIngestClient,
    telemetry::v1::TelemetrySample,
};
use tonic::transport::Channel;
use std::time::Duration;
use tokio::time::sleep;
use serial_test::serial;

fn free_port() -> u16 {
    std::net::TcpListener::bind("127.0.0.1:0").unwrap().local_addr().unwrap().port()
}

async fn wait_for_rest(port: u16) {
    let client = reqwest::Client::new();
    for _ in 0..60 { // up to ~3s
        if let Ok(r) = client.get(format!("http://127.0.0.1:{port}/health")).send().await { if r.status().is_success() { return; } }
        sleep(Duration::from_millis(50)).await;
    }
    panic!("REST server not ready on port {port}");
}

async fn connect_grpc(port: u16) -> Channel {
    for _ in 0..60 { // up to ~3s
        match Channel::from_shared(format!("http://127.0.0.1:{port}")) {
            Ok(ch) => match ch.connect().await { Ok(c) => return c, Err(_) => {} },
            Err(_) => {}
        }
        sleep(Duration::from_millis(50)).await;
    }
    panic!("gRPC server not ready on port {port}");
}

#[tokio::test]
#[serial]
async fn rest_and_grpc_basic_flow() -> anyhow::Result<()> {
    let rest_port = free_port();
    let grpc_port = free_port();

    let cfg = AppConfig {
        rest_host: "127.0.0.1".into(),
        rest_port,
        grpc_host: "127.0.0.1".into(),
        grpc_port,
        total_bandwidth_capacity_mbps: 10_000,
        kafka_brokers: None,
        kafka_telemetry_topic: "telemetry.samples".into(),
        otlp_endpoint: None,
    };

    // spawn orchestrator (background tasks)
    let shutdown = run_orchestrator(cfg).await.expect("start orchestrator");
    // Wait readiness
    wait_for_rest(rest_port).await;
    let channel = connect_grpc(grpc_port).await;

    // REST checks
    let health: serde_json::Value = reqwest::get(format!("http://127.0.0.1:{rest_port}/health")).await?.json().await?;
    assert_eq!(health["status"], "ok");

    // gRPC slice lifecycle
    let mut slice_client = SliceServiceClient::new(channel.clone());
    let create_req = GrpcSliceSpec { id: "s-basic".into(), latency_sla_ms: 40, bandwidth_mbps: 120, isolation: true };
    let resp = slice_client.create_slice(create_req.clone()).await?.into_inner();
    assert!(resp.allocated);
    assert_eq!(resp.id, "s-basic");

    let fetched = slice_client.get_slice(GrpcSliceSpec { id: "s-basic".into(), latency_sla_ms: 0, bandwidth_mbps: 0, isolation: false }).await?.into_inner();
    assert_eq!(fetched.id, "s-basic");

    // not found scenario
    let nf = slice_client.get_slice(GrpcSliceSpec { id: "does-not-exist".into(), latency_sla_ms: 0, bandwidth_mbps: 0, isolation: false }).await;
    assert!(nf.is_err());

    // Policy verification
    let mut policy_client = PolicyServiceClient::new(channel.clone());
    let verdict = policy_client.verify(Policy { name: "p1".into(), rule: "rule sample { }".into() }).await?.into_inner();
    assert!(verdict.valid);
    let invalid = policy_client.verify(Policy { name: "p2".into(), rule: "invalid rule".into() }).await?.into_inner();
    assert!(!invalid.valid);

    // Telemetry push
    let mut tele_client = TelemetryIngestClient::new(channel.clone());
    let ack = tele_client.push(TelemetrySample { slice_id: "s-basic".into(), latency_ms: 42 }).await?.into_inner();
    assert!(ack.ok);

    // trigger shutdown gracefully
    // (Not strictly necessary; drop handles.)
    drop(shutdown);
    Ok(())
}

#[tokio::test]
#[serial]
async fn grpc_capacity_enforced() -> anyhow::Result<()> {
    let rest_port = free_port();
    let grpc_port = free_port();
    let cfg = AppConfig {
        rest_host: "127.0.0.1".into(),
        rest_port,
        grpc_host: "127.0.0.1".into(),
        grpc_port,
        total_bandwidth_capacity_mbps: 150, // small capacity
        kafka_brokers: None,
        kafka_telemetry_topic: "telemetry.samples".into(),
        otlp_endpoint: None,
    };
    let _shutdown = run_orchestrator(cfg).await?;
    wait_for_rest(rest_port).await;
    let channel = connect_grpc(grpc_port).await;
    let mut slice_client = SliceServiceClient::new(channel);

    let s1 = GrpcSliceSpec { id: "cap-a".into(), latency_sla_ms: 10, bandwidth_mbps: 100, isolation: false };
    let s2 = GrpcSliceSpec { id: "cap-b".into(), latency_sla_ms: 10, bandwidth_mbps: 100, isolation: false };
    let r1 = slice_client.create_slice(s1).await?.into_inner();
    assert!(r1.allocated);
    let r2 = slice_client.create_slice(s2).await?.into_inner();
    assert!(!r2.allocated, "second allocation should fail due to capacity");
    Ok(())
}
