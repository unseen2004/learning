use serde::{Serialize, Deserialize};

#[derive(Debug, Clone, Serialize, Deserialize)]
pub struct SliceId(pub String);

#[derive(Debug, Clone, Serialize, Deserialize)]
pub struct SliceSpec {
    pub id: SliceId,
    pub latency_sla_ms: u32,
    pub bandwidth_mbps: u32,
    pub isolation: bool,
}

#[derive(Debug, Clone, Serialize, Deserialize)]
pub struct SliceStatus {
    pub id: SliceId,
    pub allocated: bool,
    pub current_bandwidth_mbps: u32,
}

