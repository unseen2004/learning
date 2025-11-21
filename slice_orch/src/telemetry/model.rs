use serde::{Serialize, Deserialize};

#[derive(Debug, Clone, Serialize, Deserialize)]
pub struct TelemetrySample { pub slice_id: String, pub latency_ms: u32 }

