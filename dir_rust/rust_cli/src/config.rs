use std::path::PathBuf;
use anyhow::{Result, anyhow, Context};
use crate::opts::Opts;

#[derive(Debug)]
pub struct Config {
    pub operation: Operation,
    pub pwd: PathBuf,
    pub config: PathBuf,
}

impl TryFrom<Opts> for Config {
    type Error = anyhow::Error;

    fn try_from(opts: Opts) -> Result<Self> {
        let operation = opts.args.try_into()?;
        let config  = get_config(opts.config)?;
        let pwd     = get_pwd(opts.pwd)?;
        Ok(Config { operation, config, pwd })
    }
}

#[derive(Debug)]
pub enum Operation {
    Print(Option<String>),
    Add(String, String),
    Remove(String),
}

impl TryFrom<Vec<String>> for Operation {
    type Error = anyhow::Error;

fn try_from(mut v: Vec<String>) -> Result<Self> {
    match v.as_slice() {
        [] => Ok(Operation::Print(None)),
        [arg] => Ok(Operation::Print(Some(arg.clone()))),
        [cmd, a, b] if cmd == "add" => Ok(Operation::Add(a.clone(), b.clone())),
        [cmd, ..] if cmd == "add" => {
            Err(anyhow!("operation `add` expects 2 args but got {}", v.len() - 1))
        }
        [cmd, a] if cmd == "rm" => Ok(Operation::Remove(a.clone())),
        [cmd, ..] if cmd == "rm" => {
            Err(anyhow!("operation `rm` expects 1 arg but got {}", v.len() - 1))
        }
        _ => Err(anyhow!("operation `print` expects at most 1 arg but got {}", v.len() - 1)),
    }
}

}

fn get_config(opt: Option<PathBuf>) -> Result<PathBuf> {
    if let Some(path) = opt {
        return Ok(path);
    }
 
let config_home = std::env::var("XDG_CONFIG_HOME")
    .context("unable to get XDG_CONFIG_HOME")?;
let mut dir = PathBuf::from(config_home);
dir.push("projector");
dir.push("projector.json");
Ok(dir)

}

fn get_pwd(opt: Option<PathBuf>) -> Result<PathBuf> {
    if let Some(path) = opt {
        Ok(path)
    } else {
        std::env::current_dir().context("error getting current_dir")
    }
}

