use crate::config::{save_global_config, load_global_config};
use anyhow::{Result, Context};
use dialoguer::Password;
use octocrab::Octocrab;
use keyring::Entry;
use tracing::{info, error, warn};

const SERVICE_NAME: &str = "repos-cli";
const USER_KEY: &str = "github_token";

pub async fn login() -> Result<()> {
    println!("Please enter your GitHub Personal Access Token (PAT).");
    println!("Permissions required: repo, read:user");
    
    let token = Password::new()
        .with_prompt("GitHub Token")
        .interact()?;

    // Validate token
    let octocrab = Octocrab::builder()
        .personal_token(token.clone())
        .build()?;

    let user = octocrab.current().user().await.context("Failed to authenticate with GitHub")?;
    
    info!("Successfully authenticated as {}", user.login);
    println!("Successfully authenticated as {}", user.login);

    // Try to save to keyring
    let mut keyring_available = false;
    if let Ok(entry) = Entry::new(SERVICE_NAME, USER_KEY) {
        if entry.set_password(&token).is_ok() {
            keyring_available = true;
            info!("Token stored in system keyring");
        }
    }

    // Always save to config file as backup
    let mut config = load_global_config()?;
    config.github_token = Some(base64::encode(&token));
    config.github_username = Some(user.login);
    save_global_config(&config)?;

    if keyring_available {
        println!("Token saved securely.");
    } else {
        println!("Token saved (keyring unavailable, using config file).");
    }
    Ok(())
}

pub fn logout() -> Result<()> {
    // Try keyring first
    if let Ok(entry) = Entry::new(SERVICE_NAME, USER_KEY) {
        match entry.delete_credential() {
            Ok(_) => info!("Token removed from keyring."),
            Err(e) => warn!("Failed to remove token from keyring: {}", e),
        }
    }

    // Clear config
    let mut config = load_global_config()?;
    config.github_token = None;
    config.github_username = None;
    save_global_config(&config)?;
    println!("Logged out successfully.");
    Ok(())
}

pub fn get_token() -> Result<String> {
    // Try keyring first
    if let Ok(entry) = Entry::new(SERVICE_NAME, USER_KEY) {
        if let Ok(password) = entry.get_password() {
            info!("Retrieved token from keyring");
            return Ok(password);
        }
    }

    // Fallback to config file
    let config = load_global_config()?;
    if let Some(encoded_token) = config.github_token {
        match base64::decode(&encoded_token) {
            Ok(decoded_bytes) => {
                match String::from_utf8(decoded_bytes) {
                    Ok(token) => {
                        info!("Retrieved token from config file");
                        return Ok(token);
                    }
                    Err(e) => error!("Failed to decode token from config: {}", e),
                }
            }
            Err(e) => error!("Failed to base64 decode token: {}", e),
        }
    }

    Err(anyhow::anyhow!("Not logged in. Please run 'repos auth login'"))
}
