use chrono::{Utc};
use serde::Serialize;
use crate::hashing::hash_bytes;
use crate::model::EventRecord;

#[derive(Serialize)]
struct EventHashView<'a> {
    ts: &'a str,
    path: &'a str,
    kind: &'a str,
    old_hash: &'a Option<String>,
    new_hash: &'a Option<String>,
    secret_count: usize,
    prev_event_hash: &'a Option<String>,
}

pub fn compute_event_hash(event: &EventRecord) -> String {
    let ts = event.ts.to_rfc3339();
    let kind = format!("{:?}", event.kind);
    let view = EventHashView { ts: &ts, path: &event.path, kind: &kind, old_hash: &event.old_hash, new_hash: &event.new_hash, secret_count: event.secret_count, prev_event_hash: &event.prev_event_hash };
    let json = serde_json::to_string(&view).unwrap_or_default();
    hash_bytes(json.as_bytes())
}

pub fn verify_chain(mut events: Vec<EventRecord>) -> bool {
    events.sort_by_key(|e| e.id.unwrap_or(0));
    let mut prev: Option<String> = None;
    for e in events.iter() {
        if e.prev_event_hash != prev { return false; }
        let h = compute_event_hash(e);
        if h != e.event_hash { return false; }
        prev = Some(h);
    }
    true
}

