use crate::telemetry::metrics;
use crate::telemetry::model::TelemetrySample;

pub async fn ingest_batch(raw: &[u8]) {
    if let Ok(text) = std::str::from_utf8(raw) {
        for line in text.lines() {
            if let Ok(sample) = serde_json::from_str::<TelemetrySample>(line) {
                metrics::record_latency(sample.latency_ms);
                ingest_sample(sample).await;
            }
        }
    }
}

pub async fn ingest_sample(_sample: TelemetrySample) { /* extend: store sample, aggregate */ }
