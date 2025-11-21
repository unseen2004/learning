use clap::{Parser, Subcommand};
use std::path::PathBuf;

#[derive(Parser, Debug)]
#[command(name="fisml", version, about="File Integrity & Secret Leakage Monitor (Enhanced)")]
pub struct Cli {
    /// Path to config TOML
    #[arg(short, long, default_value = "fisml.toml")]
    pub config: PathBuf,
    #[command(subcommand)]
    pub command: Commands,
}

#[derive(Subcommand, Debug)]
pub enum Commands {
    /// Create default config if missing
    InitConfig,
    /// Run a full baseline scan
    Baseline {
        /// Also scan for secrets during baseline
        #[arg(long)]
        secrets: bool,
    },
    /// List recent events
    ListEvents {
        /// Limit number of events
        #[arg(short, long, default_value_t = 50)]
        limit: u32,
    },
    /// Start watching for changes
    Watch {
        /// Scan modified files for secrets
        #[arg(long)]
        secrets: bool,
    },
    /// Verify hash chain integrity of all events
    VerifyChain,
    /// Export events as JSON array (stdout or file)
    ExportJson {
        /// Output file for JSON export
        #[arg(short, long)]
        output: Option<PathBuf>,
        /// Limit number of events in export
        #[arg(long)]
        limit: Option<u32>,
    },
    /// Verify signatures of all events (if signing enabled)
    VerifySignatures,
}
