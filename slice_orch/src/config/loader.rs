use super::AppConfig;
use config as cfg;
use cfg::Config;

pub fn load(path: Option<&str>) -> anyhow::Result<AppConfig> {
    let mut builder = Config::builder()
        .add_source(cfg::File::from(cfg::File::with_name("configs/default")).required(false))
        .add_source(cfg::Environment::with_prefix("ORCH").separator("__"));

    if let Some(p) = path { builder = builder.add_source(cfg::File::with_name(p).required(true)); }

    let conf = builder.build()?;
    Ok(conf.try_deserialize()?)
}

