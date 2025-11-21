use env_logger;
use log::error;
use rgrep::{run, Cli};

fn init_logging(verbosity: &clap_verbosity_flag::Verbosity) {
    let level = verbosity.log_level_filter();
    env_logger::Builder::new()
        .filter_level(level)
        .format_timestamp_secs()
        .init();
}

fn main() {
    human_panic::setup_panic!();
    let cli = Cli::parse();
    init_logging(&cli.verbose);
    match run(cli) {
        Ok(code) => std::process::exit(code),
        Err(err) => {
            error!("{:?}", err);
            std::process::exit(exitcode::SOFTWARE);
        }
    }
}