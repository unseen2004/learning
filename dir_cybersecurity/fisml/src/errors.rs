use thiserror::Error;

#[derive(Debug, Error)]
pub enum FismlError {
    #[error("Config error: {0}")]
    Config(String),
    #[error("IO error: {0}")]
    Io(#[from] std::io::Error),
    #[error("Serde TOML error: {0}")]
    TomlDe(#[from] toml::de::Error),
    #[error("Serde JSON error: {0}")]
    Json(#[from] serde_json::Error),
    #[error("SQLite error: {0}")]
    Sqlite(#[from] rusqlite::Error),
    #[error("Regex error: {0}")]
    Regex(#[from] regex::Error),
    #[error("Crypto error: {0}")]
    Crypto(String),
    #[error("Other: {0}")]
    Other(String),
}

pub type Result<T> = std::result::Result<T, FismlError>;

