use std::fs;
use std::os::unix::fs::MetadataExt;
use std::path::{PathBuf};
use walkdir::WalkDir;
use chrono::Utc;
use crate::hashing::hash_file;
use crate::model::{FileRecord, EventRecord, EventKind};
use crate::secrets::SecretScanner;
use crate::store::Store;
use crate::config::Config;
use crate::errors::Result;
use globset::GlobSet;
use crate::signing::KeyPair;
use crate::metrics;

pub struct BaselineOptions {
    pub scan_secrets: bool,
    pub keypair: Option<KeyPair>,
}

fn collect_files(paths: &[PathBuf], ignore: &GlobSet) -> Vec<PathBuf> {
    let mut out = Vec::new();
    for root in paths {
        for entry in WalkDir::new(root).into_iter().filter_map(|e| e.ok()) {
            let p = entry.path();
            if p.is_file() {
                if ignore.is_match(p) { continue; }
                out.push(p.to_path_buf());
            }
        }
    }
    out
}

pub fn run_baseline(store: &Store, cfg: &Config, secret_scanner: &SecretScanner, opts: &BaselineOptions) -> Result<Vec<EventRecord>> {
    let ignore = cfg.build_ignore_set();
    let discovered = collect_files(&cfg.paths, &ignore);
    let discovered_set: std::collections::HashSet<_> = discovered.iter().map(|p| p.to_string_lossy().to_string()).collect();

    // Deletion detection: any stored file not present now
    let mut events = Vec::new();
    for existing in store.list_files()? {
        if !discovered_set.contains(&existing) {
            if let Some(old_hash) = store.get_file_hash(&existing)? {
                let ev = EventRecord { id: None, ts: Utc::now(), path: existing.clone(), kind: EventKind::Deleted, old_hash: Some(old_hash.clone()), new_hash: None, secret_count: 0, prev_event_hash: None, event_hash: String::new(), signature: None };
                let ev = store.add_event(ev)?;
                if let Some(kp) = &opts.keypair { if cfg.enable_signing { let sig = kp.sign_hash(&ev.event_hash); store.update_signature(ev.id.unwrap(), &sig)?; } }
                metrics::inc_events(EventKind::Deleted);
                events.push(ev);
                store.delete_file(&existing)?; // remove from DB
            }
        }
    }

    for path in discovered {
        if let Ok(h) = hash_file(&path) {
            if let Ok(meta) = fs::metadata(&path) {
                let rec = FileRecord {
                    path: path.to_string_lossy().to_string(),
                    hash: h.clone(),
                    size: meta.len(),
                    mtime: meta.mtime(),
                    mode: meta.mode(),
                };
                let old = store.get_file_hash(&rec.path)?;
                store.upsert_file(&rec)?;
                let kind = match old { None => EventKind::New, Some(ref oh) if *oh != rec.hash => EventKind::Modified, Some(_) => EventKind::Baseline };
                if matches!(kind, EventKind::New | EventKind::Modified) {
                    let mut secret_count = 0usize;
                    if opts.scan_secrets {
                        if let Ok(content) = fs::read_to_string(&path) {
                            let findings = secret_scanner.scan(&content);
                            secret_count = findings.len();
                            if secret_count > 0 { metrics::inc_secrets(secret_count as u64); }
                        }
                    }
                    let ev = EventRecord { id: None, ts: Utc::now(), path: rec.path.clone(), kind: kind.clone(), old_hash: old, new_hash: Some(rec.hash.clone()), secret_count, prev_event_hash: None, event_hash: String::new(), signature: None };
                    let ev = store.add_event(ev)?;
                    if let Some(kp) = &opts.keypair { if cfg.enable_signing { let sig = kp.sign_hash(&ev.event_hash); store.update_signature(ev.id.unwrap(), &sig)?; } }
                    metrics::inc_events(kind.clone());
                    events.push(ev);
                }
            }
        }
    }
    Ok(events)
}
