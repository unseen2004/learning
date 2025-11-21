use criterion::{criterion_group, criterion_main, Criterion};
use slice_orchestrator::{types::{SliceSpec, SliceId}, controller::allocator};

fn bench_alloc(c: &mut Criterion) {
    c.bench_function("allocate", |b| {
        b.iter(|| {
            let spec = SliceSpec { id: SliceId("b".into()), latency_sla_ms: 10, bandwidth_mbps: 50, isolation: false };
            let _ = allocator::allocate(&spec);
        })
    });
}

criterion_group!(name = benches; config = Criterion::default(); targets = bench_alloc);
criterion_main!(benches);
use thiserror::Error;

#[derive(Debug, Error)]
pub enum OrchestratorError {
    #[error("configuration error: {0}")]
    Config(String),
    #[error("not found: {0}")]
    NotFound(String),
    #[error("internal error: {0}")]
    Internal(String),
}

pub type Result<T> = std::result::Result<T, OrchestratorError>;

