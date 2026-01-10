use clap::{Parser, Subcommand};
use std::process::Command;
use std::path::Path;
use anyhow::{Context, Result, anyhow};

#[derive(Parser)]
#[command(name = "stereorepo")]
#[command(about = "Manage stereorepo sparse checkouts", long_about = None)]
struct Cli {
    #[command(subcommand)]
    command: Commands,
}

#[derive(Subcommand)]
enum Commands {
    /// Clone a repository with sparse checkout
    Clone {
        /// Repository URL
        url: String,
        /// Destination directory
        path: String,
        /// Comma-separated list of modules/directories to include
        #[arg(short, long, value_delimiter = ',', num_args = 1..)]
        modules: Vec<String>,
    },
    /// Add modules to the current sparse checkout
    Add {
        /// Modules to add
        #[arg(num_args = 1..)]
        modules: Vec<String>,
    },
    /// List current sparse checkout rules
    List,
}

fn main() -> Result<()> {
    let cli = Cli::parse();

    match &cli.command {
        Commands::Clone { url, path, modules } => {
            clone_repo(url, path, modules)?;
        }
        Commands::Add { modules } => {
            add_modules(modules)?;
        }
        Commands::List => {
            list_modules()?;
        }
    }

    Ok(())
}

fn clone_repo(url: &str, path: &str, modules: &[String]) -> Result<()> {
    println!("Cloning {} into {}...", url, path);
    
    let status = Command::new("git")
        .args(["clone", "--filter=blob:none", "--no-checkout", url, path])
        .status()
        .context("Failed to execute git clone")?;

    if !status.success() {
        return Err(anyhow!("Git clone failed"));
    }

    let repo_path = Path::new(path);
    
    let status = Command::new("git")
        .current_dir(repo_path)
        .args(["sparse-checkout", "init", "--cone"])
        .status()
        .context("Failed to init sparse-checkout")?;

     if !status.success() {
        return Err(anyhow!("Git sparse-checkout init failed"));
    }

    if !modules.is_empty() {
        set_sparse_checkout(repo_path, modules)?;
    }

    println!("Checking out files...");
    let status = Command::new("git")
        .current_dir(repo_path)
        .arg("checkout")
        .status()
        .context("Failed to checkout")?;

    if !status.success() {
        return Err(anyhow!("Git checkout failed"));
    }

    Ok(())
}

fn add_modules(modules: &[String]) -> Result<()> {
    let status = Command::new("git")
        .args(["sparse-checkout", "add"])
        .args(modules)
        .status()
        .context("Failed to execute git sparse-checkout add")?;
    
    if !status.success() {
        return Err(anyhow!("Git sparse-checkout add failed"));
    }
    
    // We often need to re-checkout or read-tree to apply changes if they didn't appear immediately,
    // but sparse-checkout add usually updates the working directory immediately in modern git versions.
    // Explicit checkout ensures state.
    let status = Command::new("git")
        .arg("checkout")
        .status()
        .context("Failed to update checkout")?;

    if !status.success() {
        return Err(anyhow!("Git checkout failed"));
    }

    Ok(())
}

fn set_sparse_checkout(path: &Path, modules: &[String]) -> Result<()> {
    let status = Command::new("git")
        .current_dir(path)
        .args(["sparse-checkout", "set"])
        .args(modules)
        .status()
        .context("Failed to set sparse modules")?;

    if !status.success() {
        return Err(anyhow!("Failed to set sparse modules"));
    }
    Ok(())
}

fn list_modules() -> Result<()> {
    let status = Command::new("git")
        .args(["sparse-checkout", "list"])
        .status()
        .context("Failed to list sparse modules")?;

    if !status.success() {
        return Err(anyhow!("Failed to list modules"));
    }
    Ok(())
}