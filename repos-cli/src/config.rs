use serde::{Deserialize, Serialize};
use std::path::PathBuf;
use anyhow::{Result, Context};
use directories::ProjectDirs;
use std::fs;

#[derive(Debug, Serialize, Deserialize, Default)]
pub struct GlobalConfig {
    pub github_username: Option<String>,
    #[serde(default)]
    pub github_token: Option<String>, // Fallback if keyring unavailable
    #[serde(default)]
    pub tracked_repos: Vec<PathBuf>,
}

#[derive(Debug, Serialize, Deserialize, Default)]
pub struct LocalConfig {
    pub auto_update: bool,
    pub features: Features,
}

#[derive(Debug, Serialize, Deserialize)]
pub struct Features {
    pub badges: bool,
    pub emoji: bool,
    pub changelog: bool,
    pub dependencies: bool,
    pub docker: bool,
    pub categorizer: bool,
}

impl Default for Features {
    fn default() -> Self {
        Self {
            badges: true,
            emoji: true,
            changelog: true,
            dependencies: true,
            docker: true,
            categorizer: true,
        }
    }
}

pub fn get_config_dir() -> Result<PathBuf> {
    let proj_dirs = ProjectDirs::from("com", "repos-cli", "repos")
        .context("Could not determine config directory")?;
    Ok(proj_dirs.config_dir().to_path_buf())
}

pub fn load_global_config() -> Result<GlobalConfig> {
    let config_dir = get_config_dir()?;
    let config_path = config_dir.join("config.toml");

    if !config_path.exists() {
        return Ok(GlobalConfig::default());
    }

    let content = fs::read_to_string(config_path)?;
    let config: GlobalConfig = toml::from_str(&content)?;
    Ok(config)
}

pub fn save_global_config(config: &GlobalConfig) -> Result<()> {
    let config_dir = get_config_dir()?;
    fs::create_dir_all(&config_dir)?;
    let config_path = config_dir.join("config.toml");
    
    let content = toml::to_string_pretty(config)?;
    fs::write(config_path, content)?;
    Ok(())
}

pub fn add_repo(path: PathBuf) -> Result<()> {
    let path = fs::canonicalize(path).context("Failed to resolve path")?;
    
    if !path.exists() || !path.is_dir() {
        anyhow::bail!("Path does not exist or is not a directory: {:?}", path);
    }

    let mut config = load_global_config()?;
    
    if config.tracked_repos.contains(&path) {
        println!("Repository is already tracked: {:?}", path);
        return Ok(());
    }

    config.tracked_repos.push(path.clone());
    save_global_config(&config)?;
    
    println!("Added repository to tracking list: {:?}", path);
    Ok(())
}

pub fn remove_repo(path: PathBuf) -> Result<()> {
    let path = fs::canonicalize(path).context("Failed to resolve path")?;
    
    let mut config = load_global_config()?;
    
    if let Some(pos) = config.tracked_repos.iter().position(|p| p == &path) {
        config.tracked_repos.remove(pos);
        save_global_config(&config)?;
        println!("Removed repository from tracking list: {:?}", path);
    } else {
        println!("Repository is not being tracked: {:?}", path);
    }
    
    Ok(())
}

pub fn list_tracked_repos() -> Result<()> {
    let config = load_global_config()?;
    
    if config.tracked_repos.is_empty() {
        println!("No tracked repositories.");
        return Ok(());
    }
    
    println!("Tracked repositories:");
    for (i, repo) in config.tracked_repos.iter().enumerate() {
        let status = if repo.exists() { "✓" } else { "✗" };
        println!("  {} [{}] {:?}", i + 1, status, repo);
    }
    
    Ok(())
}
