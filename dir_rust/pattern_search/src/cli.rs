use clap::{ArgAction, Parser};
use clap_verbosity_flag::Verbosity;
use std::path::PathBuf;

#[derive(Parser, Debug, Clone)]
#[command(name = "rgrep", author = "unseen", version = "1.0.0", about = "Fast, idiomatic Rust grep-like CLI")]
pub struct Cli {
    pub pattern: String,
    pub paths: Vec<PathBuf>,
    #[arg(short = 'i', long = "ignore-case", action = ArgAction::SetTrue)]
    pub ignore_case: bool,
    #[arg(long = "invert-match", action = ArgAction::SetTrue)]
    pub invert_match: bool,
    #[arg(short = 'n', long = "line-number", action = ArgAction::SetTrue)]
    pub line_number: bool,
    #[arg(short = 'c', long = "count", action = ArgAction::SetTrue)]
    pub count: bool,
    #[arg(long = "json", action = ArgAction::SetTrue)]
    pub json: bool,
    #[command(flatten)]
    pub verbose: Verbosity,
}

impl Cli {
    pub fn parse() -> Self {
        <Self as clap::Parser>::parse()
    }
}