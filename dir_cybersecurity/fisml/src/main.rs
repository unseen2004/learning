mod errors;
mod config;
mod model;
mod hashing;
mod secrets;
mod logchain;
mod signing;
mod store;
mod baseline;
mod watcher;
mod cli;
mod metrics;

use clap::Parser;
use tracing_subscriber::EnvFilter;
use crate::cli::{Cli, Commands};
use crate::config::Config;
use crate::errors::Result;
use crate::store::Store;
use crate::secrets::SecretScanner;
use crate::baseline::{run_baseline, BaselineOptions};
use crate::signing::KeyPair;
use std::net::SocketAddr;

fn main() {
    if let Err(e) = real_main() { eprintln!("Error: {e}"); std::process::exit(1); }
}

fn real_main() -> Result<()> {
    tracing_subscriber::fmt().with_env_filter(EnvFilter::from_default_env()).init();
    let cli = Cli::parse();
    let cfg_path = cli.config.clone();
    let cfg = Config::load_or_create(&cfg_path)?;
    let store = Store::open(&cfg.db_path)?;
    let scanner = SecretScanner::new(cfg.entropy_threshold);

    // Initialize keypair if signing enabled
    let keypair = if cfg.enable_signing { cfg.signing_key.as_ref().and_then(|p| KeyPair::load_or_generate(p).ok()) } else { None };

    // Spawn metrics server if enabled
    if cfg.enable_metrics {
        if let Some(addr) = cfg.metrics_bind.as_ref() {
            if let Ok(sock) = addr.parse::<SocketAddr>() { metrics::spawn_metrics(sock); }
        }
    }

    match cli.command {
        Commands::InitConfig => {
            println!("Config written to {}", cfg_path.display());
        }
        Commands::Baseline { secrets } => {
            let events = run_baseline(&store, &cfg, &scanner, &BaselineOptions { scan_secrets: secrets, keypair: keypair.clone() })?;
            println!("Baseline complete. New/modified/deleted events: {}", events.len());
        }
        Commands::ListEvents { limit } => {
            let events = store.list_events(limit as usize)?;
            for e in events { println!("{} {} {:?} {} secrets={} hash={} sig={}", e.id.unwrap_or(0), e.ts, e.kind, e.path, e.secret_count, e.event_hash, e.signature.as_ref().map(|_| "yes").unwrap_or("no")); }
        }
        Commands::Watch { secrets } => {
            watcher::run_watcher(&store, &cfg, &scanner, secrets, keypair.clone())?;
        }
        Commands::VerifyChain => {
            let events = store.all_events()?;
            let ok = logchain::verify_chain(events.clone());
            if ok { println!("Chain OK ({} events)", events.len()); } else { println!("Chain INVALID"); }
        }
        Commands::ExportJson { output, limit } => {
            let json = if let Some(l) = limit { serde_json::to_string_pretty(&store.list_events(l as usize)?)? } else { serde_json::to_string_pretty(&store.all_events()?)? };
            if let Some(path) = output { std::fs::write(&path, &json)?; println!("Exported JSON to {}", path.display()); } else { println!("{json}"); }
        }
        Commands::VerifySignatures => {
            if !cfg.enable_signing { println!("Signing not enabled in config"); }
            let Some(kp) = keypair else { println!("No keypair loaded"); return Ok(()); };
            let mut ok_count = 0usize; let mut bad: Vec<i64> = Vec::new();
            for ev in store.all_events()? { if let Some(sig) = ev.signature.clone() { if kp.verify_hash(&ev.event_hash, &sig) { ok_count += 1; } else { bad.push(ev.id.unwrap_or(-1)); } } }
            if bad.is_empty() { println!("All signatures valid ({}).", ok_count); } else { println!("Invalid signatures for event IDs: {:?}", bad); }
        }
    }
    Ok(())
}
