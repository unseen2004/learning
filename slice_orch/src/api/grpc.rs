#![allow(clippy::all)]
#[cfg(feature = "grpc")]
use crate::{types::{SliceSpec as DomSliceSpec, SliceId, SliceStatus as DomSliceStatus}, controller::allocator, store, policy};
#[cfg(feature = "grpc")]
use std::net::SocketAddr;
#[cfg(feature = "grpc")]
use tonic::{Request, Response, Status};

#[cfg(feature = "grpc")]
pub mod pb {
    pub mod sliceorchestrator {
        pub mod slice { pub mod v1 { tonic::include_proto!("sliceorchestrator.slice.v1"); } }
        pub mod telemetry { pub mod v1 { tonic::include_proto!("sliceorchestrator.telemetry.v1"); } }
        pub mod policy { pub mod v1 { tonic::include_proto!("sliceorchestrator.policy.v1"); } }
    }
}

#[cfg(feature = "grpc")]
use pb::sliceorchestrator::slice::v1::{slice_service_server::{SliceService, SliceServiceServer}, SliceSpec, SliceStatus};
#[cfg(feature = "grpc")]
use pb::sliceorchestrator::telemetry::v1::{telemetry_ingest_server::{TelemetryIngest, TelemetryIngestServer}, TelemetrySample, Ack};
#[cfg(feature = "grpc")]
use pb::sliceorchestrator::policy::v1::{policy_service_server::{PolicyService, PolicyServiceServer}, Policy, PolicyVerdict};

#[cfg(feature = "grpc")]
#[derive(Default, Clone)]
struct SliceSvc;
#[cfg(feature = "grpc")]
#[derive(Default, Clone)]
struct TelemetrySvc;
#[cfg(feature = "grpc")]
#[derive(Default, Clone)]
struct PolicySvc;

#[cfg(feature = "grpc")]
fn to_domain(spec: &SliceSpec) -> DomSliceSpec {
    DomSliceSpec { id: SliceId(spec.id.clone()), latency_sla_ms: spec.latency_sla_ms, bandwidth_mbps: spec.bandwidth_mbps, isolation: spec.isolation }
}
#[cfg(feature = "grpc")]
fn to_proto_status(st: &DomSliceStatus) -> SliceStatus { SliceStatus { id: st.id.0.clone(), allocated: st.allocated, current_bandwidth_mbps: st.current_bandwidth_mbps } }

#[cfg(feature = "grpc")]
#[tonic::async_trait]
impl SliceService for SliceSvc {
    async fn create_slice(&self, req: Request<SliceSpec>) -> Result<Response<SliceStatus>, Status> {
        let spec = req.into_inner();
        let d = to_domain(&spec);
        let status = allocator::allocate(&d);
        store::state::put(&d, status.clone());
        Ok(Response::new(to_proto_status(&status)))
    }
    async fn get_slice(&self, req: Request<SliceSpec>) -> Result<Response<SliceStatus>, Status> {
        let spec = req.into_inner();
        let id = SliceId(spec.id);
        if let Some(status) = store::state::get(&id) { return Ok(Response::new(to_proto_status(&status))); }
        Err(Status::not_found("slice not found"))
    }
}

#[cfg(feature = "grpc")]
#[tonic::async_trait]
impl TelemetryIngest for TelemetrySvc {
    async fn push(&self, req: Request<TelemetrySample>) -> Result<Response<Ack>, Status> {
        let sample = req.into_inner();
        tracing::debug!(slice_id=%sample.slice_id, latency_ms=%sample.latency_ms, "ingest grpc telemetry sample");
        Ok(Response::new(Ack { ok: true }))
    }
}

#[cfg(feature = "grpc")]
#[tonic::async_trait]
impl PolicyService for PolicySvc {
    async fn verify(&self, req: Request<Policy>) -> Result<Response<PolicyVerdict>, Status> {
        let pol = req.into_inner();
        let parse_ok = crate::policy::dsl::parse_rule(&pol.rule).is_ok();
        let valid = parse_ok && policy::verifier::verify_policies(&[pol.rule.clone()]);
        let verdict = PolicyVerdict { name: pol.name, valid, reason: if valid { String::new() } else { "invalid policy".into() } };
        Ok(Response::new(verdict))
    }
}

#[cfg(feature = "grpc")]
pub async fn spawn_grpc(addr: SocketAddr) -> anyhow::Result<tokio::task::JoinHandle<()>> {
    use tonic::transport::Server;
    let slice_svc = SliceSvc::default();
    let tel_svc = TelemetrySvc::default();
    let pol_svc = PolicySvc::default();
    tracing::info!(%addr, "gRPC server listening");
    let handle = tokio::spawn(async move {
        if let Err(e) = Server::builder()
            .add_service(SliceServiceServer::new(slice_svc))
            .add_service(TelemetryIngestServer::new(tel_svc))
            .add_service(PolicyServiceServer::new(pol_svc))
            .serve(addr)
            .await { tracing::error!(error=%e, "gRPC server exit"); }
    });
    Ok(handle)
}

#[cfg(not(feature = "grpc"))]
pub async fn spawn_grpc(_addr: std::net::SocketAddr) -> anyhow::Result<tokio::task::JoinHandle<()>> { anyhow::bail!("grpc feature disabled") }
