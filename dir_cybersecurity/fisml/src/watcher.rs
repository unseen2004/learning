use std::path::{PathBuf};
use std::sync::{mpsc, Arc, atomic::{AtomicBool, Ordering}};
use notify::{RecommendedWatcher, RecursiveMode, Watcher};
use chrono::Utc;
use crate::store::Store;
use crate::config::Config;
use crate::secrets::SecretScanner;
use crate::hashing::hash_file;
use crate::model::{EventRecord, EventKind as FKind};
use crate::errors::Result;
use std::os::unix::fs::MetadataExt;
use crate::signing::KeyPair;
use crate::metrics;

pub fn run_watcher(store: &Store, cfg: &Config, scanner: &SecretScanner, scan_secrets: bool, keypair: Option<KeyPair>) -> Result<()> {
    println!("Watching for changes. Press Ctrl+C to stop.");
    let ignore = cfg.build_ignore_set();
    let signing_enabled = cfg.enable_signing && keypair.is_some();
    let running = Arc::new(AtomicBool::new(true));
    {
        let r = running.clone();
        ctrlc::set_handler(move || { r.store(false, Ordering::SeqCst); }).ok();
    }

    let (tx, rx) = mpsc::channel::<PathBuf>();
    let mut watcher: RecommendedWatcher = Watcher::new(move |res: notify::Result<notify::Event>| {
        if let Ok(event) = res {
            for path in event.paths { let _ = tx.send(path); }
        }
    }, notify::Config::default()).map_err(|e| crate::errors::FismlError::Other(e.to_string()))?;

    for p in &cfg.paths { watcher.watch(p, RecursiveMode::Recursive).map_err(|e| crate::errors::FismlError::Other(e.to_string()))?; }

    while running.load(Ordering::SeqCst) {
        if let Ok(path) = rx.recv_timeout(std::time::Duration::from_millis(500)) {
            if !path.is_file() { continue; }
            if ignore.is_match(&path) { continue; }
            let sp = path.to_string_lossy().to_string();
            match process_path(store, &sp, scan_secrets, scanner) {
                Ok(Some(ev)) => {
                    if signing_enabled {
                        if let Some(kp) = &keypair { let sig = kp.sign_hash(&ev.event_hash); store.update_signature(ev.id.unwrap(), &sig)?; }
                    }
                    metrics::inc_events(ev.kind.clone());
                    if ev.secret_count > 0 { metrics::inc_secrets(ev.secret_count as u64); }
                    println!("EVENT {:?} {} secrets={} hash={} ", ev.kind, ev.path, ev.secret_count, ev.event_hash);
                }
                Ok(None) => {}
                Err(e) => eprintln!("Process error: {e}")
            }
        }
    }
    println!("Watcher stopped.");
    Ok(())
}

fn process_path(store: &Store, path: &str, scan_secrets: bool, scanner: &SecretScanner) -> Result<Option<EventRecord>> {
    let p = std::path::Path::new(path);
    let new_hash = match hash_file(p) { Ok(h) => h, Err(_) => return Ok(None) };
    let old = store.get_file_hash(path)?;
    let kind = match old { None => FKind::New, Some(ref oh) if *oh != new_hash => FKind::Modified, Some(_) => return Ok(None) };
    let mut secret_count = 0usize;
    if scan_secrets && p.metadata().map(|m| m.len()).unwrap_or(0) < 5_000_000 { // size guard
        if let Ok(content) = std::fs::read_to_string(p) {
            secret_count = scanner.scan(&content).len();
        }
    }
    let rec = crate::model::FileRecord { path: path.to_string(), hash: new_hash.clone(), size: p.metadata().map(|m| m.len()).unwrap_or(0), mtime: p.metadata().map(|m| m.mtime()).unwrap_or(0), mode: p.metadata().map(|m| m.mode()).unwrap_or(0) };
    store.upsert_file(&rec)?;
    let ev = EventRecord { id: None, ts: Utc::now(), path: path.to_string(), kind, old_hash: old, new_hash: Some(new_hash), secret_count, prev_event_hash: None, event_hash: String::new(), signature: None };
    let ev = store.add_event(ev)?;
    Ok(Some(ev))
}
