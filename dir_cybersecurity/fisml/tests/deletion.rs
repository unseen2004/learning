use tempfile::tempdir;
use std::fs;
use r::config::Config;
use r::store::Store;
use r::secrets::SecretScanner;
use r::baseline::{run_baseline, BaselineOptions};

#[test]
fn test_deletion_detection() {
    let dir = tempdir().unwrap();
    let root = dir.path();
    let db_path = root.join("fisml.db");
    let file_a = root.join("a.txt");
    let file_b = root.join("b.txt");
    fs::write(&file_a, b"alpha").unwrap();
    fs::write(&file_b, b"beta").unwrap();

    let cfg = Config { paths: vec![root.to_path_buf()], ignore_globs: vec![], entropy_threshold: 4.0, signing_key: None, db_path: db_path.clone(), enable_signing: false, enable_metrics: false, metrics_bind: None, api_bind: None };
    let store = Store::open(&cfg.db_path).unwrap();
    let scanner = SecretScanner::new(cfg.entropy_threshold);

    // Initial baseline
    let ev1 = run_baseline(&store, &cfg, &scanner, &BaselineOptions { scan_secrets: false, keypair: None }).unwrap();
    assert!(ev1.iter().any(|e| e.path.ends_with("a.txt")));
    assert!(ev1.iter().any(|e| e.path.ends_with("b.txt")));

    // Delete one file
    fs::remove_file(&file_b).unwrap();

    // Second baseline should record deletion
    let ev2 = run_baseline(&store, &cfg, &scanner, &BaselineOptions { scan_secrets: false, keypair: None }).unwrap();
    let deleted = ev2.iter().any(|e| e.kind == r::model::EventKind::Deleted && e.path.ends_with("b.txt"));
    assert!(deleted, "Expected deletion event for b.txt");
}

