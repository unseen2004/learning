mod auth;
mod config;
mod github;
mod generators;
mod readme;

use clap::{Parser, Subcommand};
use anyhow::Result;
use std::path::PathBuf;

#[derive(Parser)]
#[command(author, version, about, long_about = None)]
struct Cli {
    #[command(subcommand)]
    command: Commands,
}

#[derive(Subcommand)]
enum Commands {
    /// Authentication management
    Auth {
        #[command(subcommand)]
        command: AuthCommands,
    },
    /// List repositories
    List,
    /// Add a repository to track
    Add {
        /// Path to the repository
        path: PathBuf,
    },
    /// Remove a repository from tracking
    Remove {
        /// Path to the repository
        path: PathBuf,
    },
    /// Show all tracked repositories
    Tracked,
    /// Update READMEs for tracked repositories
    Update {
        /// Optional path to update specific repo (must be tracked)
        path: Option<PathBuf>,
    },
}

#[derive(Subcommand)]
enum AuthCommands {
    /// Login to GitHub
    Login,
    /// Logout from GitHub
    Logout,
}

#[tokio::main]
async fn main() -> Result<()> {
    tracing_subscriber::fmt::init();
    let cli = Cli::parse();

    match cli.command {
        Commands::Auth { command } => match command {
            AuthCommands::Login => auth::login().await?,
            AuthCommands::Logout => auth::logout()?,
        },
        Commands::List => {
            github::list_repos().await?;
        }
        Commands::Add { path } => {
            config::add_repo(path)?;
        }
        Commands::Remove { path } => {
            config::remove_repo(path)?;
        }
        Commands::Tracked => {
            config::list_tracked_repos()?;
        }
        Commands::Update { path } => {
            if let Some(p) = path {
                readme::update_readme(&p)?;
            } else {
                let config = config::load_global_config()?;
                for repo in config.tracked_repos {
                    if repo.exists() {
                        readme::update_readme(&repo)?;
                    }
                }
            }
        }
    }

    Ok(())
}
