use std::path::PathBuf;
use clap::Parser;

#[derive(Parser, Debug)]
#[clap(author, version, about)]
pub struct Opts {
    pub args: Vec<String>,

    #[clap(short = 'c', long)]
    pub config: Option<PathBuf>,

    #[clap(short = 'p', long)]
    pub pwd: Option<PathBuf>,
}

