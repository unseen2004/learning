use std::fs;
use tempfile::tempdir;
use r::store::Store;
use r::config::Config;
use r::secrets::SecretScanner;
use r::baseline::{run_baseline, BaselineOptions};
use r::logchain;

#[test]
fn test_chain_verification() {
    let dir = tempdir().expect("tempdir");
    let root = dir.path();
    let db_path = root.join("fisml.db");
    let file_path = root.join("sample.txt");
    fs::write(&file_path, b"Example content AKIAABCDEFGHIJKLMNOP secret").expect("write sample");

    let cfg = Config {
        paths: vec![root.to_path_buf()],
        ignore_globs: vec![],
        entropy_threshold: 4.0,
        signing_key: None,
        db_path: db_path.clone(),
        enable_signing: false,
        enable_metrics: false,
        metrics_bind: None,
        api_bind: None,
    };

    let store = Store::open(&cfg.db_path).expect("open store");
    let scanner = SecretScanner::new(cfg.entropy_threshold);

    let events_created = run_baseline(&store, &cfg, &scanner, &BaselineOptions { scan_secrets: true, keypair: None })
        .expect("baseline");
    assert!(!events_created.is_empty(), "Expected at least one event from baseline");

    let all = store.all_events().expect("all events");
    assert!(logchain::verify_chain(all.clone()), "Chain should verify after baseline");

    let mut tampered = all.clone();
    if let Some(first) = tampered.first_mut() { first.prev_event_hash = Some("bad".into()); }
    assert!(!logchain::verify_chain(tampered), "Tampered chain should fail verification");
}
