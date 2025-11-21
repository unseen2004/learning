#[cfg(feature = "otel")]
use opentelemetry::{global, metrics::{Counter, Histogram}};
use std::sync::OnceLock;

static ALLOC_BW_COUNTER: OnceLock<AllocMetrics> = OnceLock::new();

struct AllocMetrics { #[allow(dead_code)] bytes_counter: Option<Counter<u64>>, latency_ms: Option<Histogram<u64>> }

#[cfg(feature = "otel")]
fn build() -> AllocMetrics {
    let meter = global::meter("slice-orchestrator");
    let bytes_counter = meter.u64_counter("slice_allocated_bandwidth_mbps").with_description("Allocated bandwidth (mbps) cumulative").init();
    let latency_ms = meter.u64_histogram("slice_latency_ms").with_description("Observed latency per sample").init();
    AllocMetrics { bytes_counter: Some(bytes_counter), latency_ms: Some(latency_ms) }
}
#[cfg(not(feature = "otel"))]
fn build() -> AllocMetrics { AllocMetrics { bytes_counter: None, latency_ms: None } }

fn ensure() -> &'static AllocMetrics { ALLOC_BW_COUNTER.get_or_init(build) }

pub fn record_alloc(bw_mbps: u32) { if let Some(c) = &ensure().bytes_counter { c.add(bw_mbps as u64, &[]); } }
pub fn record_latency(latency_ms: u32) { if let Some(h) = &ensure().latency_ms { h.record(latency_ms as u64, &[]); } }
