use serde::{Serialize, Deserialize};
use chrono::{DateTime, Utc};

#[derive(Debug, Clone, Serialize, Deserialize, PartialEq, Eq)]
pub enum EventKind {
    Baseline,
    New,
    Modified,
    Deleted,
}

#[derive(Debug, Clone, Serialize, Deserialize)]
pub struct FileRecord {
    pub path: String,
    pub hash: String,
    pub size: u64,
    pub mtime: i64,
    pub mode: u32,
}

#[derive(Debug, Clone, Serialize, Deserialize)]
pub struct SecretFinding {
    pub kind: String,
    pub span: (usize, usize),
    pub entropy: f64,
    pub preview_hash: String,
}

#[derive(Debug, Clone, Serialize, Deserialize)]
pub struct EventRecord {
    pub id: Option<i64>,
    pub ts: DateTime<Utc>,
    pub path: String,
    pub kind: EventKind,
    pub old_hash: Option<String>,
    pub new_hash: Option<String>,
    pub secret_count: usize,
    pub prev_event_hash: Option<String>,
    pub event_hash: String,
    pub signature: Option<Vec<u8>>,
}

