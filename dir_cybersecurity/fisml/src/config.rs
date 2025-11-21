use std::path::{PathBuf};
use std::fs;
use serde::{Deserialize, Serialize};
use crate::errors::{Result, FismlError};
use globset::{Glob, GlobSet, GlobSetBuilder};

#[derive(Debug, Clone, Serialize, Deserialize)]
pub struct Config {
    pub paths: Vec<PathBuf>,
    #[serde(alias = "ignore")] // backward compatibility
    pub ignore_globs: Vec<String>,
    pub entropy_threshold: f64,
    pub signing_key: Option<PathBuf>,
    pub db_path: PathBuf,
    #[serde(default)]
    pub enable_signing: bool,
    #[serde(default)]
    pub enable_metrics: bool,
    #[serde(default)]
    pub metrics_bind: Option<String>,
    #[serde(default)]
    pub api_bind: Option<String>,
}

impl Default for Config {
    fn default() -> Self {
        Self { 
            paths: vec![PathBuf::from("." )],
            ignore_globs: vec!["target/**".into(), ".git/**".into()],
            entropy_threshold: 4.0,
            signing_key: Some(PathBuf::from("fisml.key")),
            db_path: PathBuf::from("fisml.db"),
            enable_signing: false,
            enable_metrics: true,
            metrics_bind: Some("127.0.0.1:9898".into()),
            api_bind: None,
        }
    }
}

impl Config {
    pub fn load_or_create(path: &PathBuf) -> Result<Self> {
        if path.exists() {
            let txt = fs::read_to_string(path)?;
            let mut cfg: Config = toml::from_str(&txt)?;
            // ensure defaults for new fields
            if cfg.signing_key.is_none() { cfg.signing_key = Some(PathBuf::from("fisml.key")); }
            Ok(cfg)
        } else {
            let cfg = Config::default();
            let serialized = toml::to_string_pretty(&cfg).map_err(|e| FismlError::Config(e.to_string()))?;
            fs::write(path, serialized)?;
            Ok(cfg)
        }
    }

    pub fn build_ignore_set(&self) -> GlobSet {
        let mut builder = GlobSetBuilder::new();
        for g in &self.ignore_globs {
            if let Ok(glob) = Glob::new(g) { builder.add(glob); }
        }
        builder.build().unwrap_or_else(|_| GlobSetBuilder::new().build().unwrap())
    }
}
