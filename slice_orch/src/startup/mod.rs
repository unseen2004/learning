mod wiring;
use tracing::info;

pub use wiring::run_orchestrator;
pub use crate::config::AppConfig; // re-export for convenience

pub fn banner() {
    info!("starting slice orchestrator");
}
