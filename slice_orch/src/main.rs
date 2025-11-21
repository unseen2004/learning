// Orchestrator binary entrypoint
use slice_orchestrator::startup::run_orchestrator;
use slice_orchestrator::startup::AppConfig;
use tracing_subscriber::{EnvFilter, fmt};

#[tokio::main]
async fn main() -> anyhow::Result<()> {
    // Load config early to decide tracing strategy
    let cfg = AppConfig::load(None)?;

    let otel_enabled = cfg!(feature = "otel") && cfg.otel_enabled();
    if !otel_enabled {
        let json = std::env::var("RUST_LOG_JSON").ok().filter(|v| v == "1").is_some();
        let filter = EnvFilter::try_from_default_env().unwrap_or_else(|_| EnvFilter::new("info"));
        if json {
            fmt().with_env_filter(filter).json().init();
        } else {
            fmt().with_env_filter(filter).compact().init();
        }
    }

    let shutdown = run_orchestrator(cfg).await?;
    // Wait for ctrl+c
    shutdown.await;
    Ok(())
}
