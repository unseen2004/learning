pub mod cli;
pub mod engine;
use anyhow::Result;
pub use cli::Cli;

pub fn run(cli: Cli) -> Result<i32> {
    engine::run(cli)
}