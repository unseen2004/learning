use tempfile::tempdir;
use std::fs;
use r::config::Config;
use r::store::Store;
use r::secrets::SecretScanner;
use r::baseline::{run_baseline, BaselineOptions};
use r::signing::KeyPair;

#[test]
fn test_event_signatures() {
    let dir = tempdir().unwrap();
    let root = dir.path();
    let db_path = root.join("fisml.db");
    let key_path = root.join("fisml.key");
    let file_path = root.join("secret.txt");
    fs::write(&file_path, b"TOPSECRET_TOKEN_1234567890ABCDEFGHIJ").unwrap();

    let mut cfg = Config::default();
    cfg.paths = vec![root.to_path_buf()];
    cfg.db_path = db_path.clone();
    cfg.signing_key = Some(key_path.clone());
    cfg.enable_signing = true;
    cfg.enable_metrics = false;
    cfg.ignore_globs = vec![];

    let store = Store::open(&cfg.db_path).unwrap();
    let scanner = SecretScanner::new(cfg.entropy_threshold);
    let kp = KeyPair::load_or_generate(cfg.signing_key.as_ref().unwrap()).unwrap();

    let events = run_baseline(&store, &cfg, &scanner, &BaselineOptions { scan_secrets: true, keypair: Some(kp.clone()) }).unwrap();
    assert!(!events.is_empty(), "Expected events with signing");

    // Fetch all events and ensure signatures are present and valid
    for ev in store.all_events().unwrap() {
        if let Some(sig) = ev.signature.clone() {
            assert!(kp.verify_hash(&ev.event_hash, &sig), "Signature should verify");
        }
    }
}

