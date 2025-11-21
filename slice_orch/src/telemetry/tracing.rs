#[cfg(feature = "otel")]
pub fn init_tracing(cfg: &crate::config::AppConfig) -> anyhow::Result<()> {
    use opentelemetry::{KeyValue, trace::TracerProvider};
    use opentelemetry_sdk::{trace as sdktrace, Resource};
    use opentelemetry_otlp::WithExportConfig;
    use tracing_subscriber::{layer::SubscriberExt, util::SubscriberInitExt};

    if !cfg.otel_enabled() { return Ok(()); }

    let endpoint = cfg.otlp_endpoint.clone().unwrap();
    let provider = opentelemetry_otlp::new_pipeline()
        .tracing()
        .with_trace_config(sdktrace::Config::default().with_resource(Resource::new(vec![
            KeyValue::new("service.name", "slice-orchestrator"),
        ])))
        .with_exporter(opentelemetry_otlp::new_exporter().tonic().with_endpoint(endpoint))
        .install_batch(opentelemetry_sdk::runtime::Tokio)?;

    let tracer = provider.tracer("slice-orchestrator");
    let otel_layer = tracing_opentelemetry::layer().with_tracer(tracer);
    let filter = tracing_subscriber::EnvFilter::try_from_default_env().unwrap_or_else(|_| tracing_subscriber::EnvFilter::new("info"));
    tracing_subscriber::registry()
        .with(filter)
        .with(tracing_subscriber::fmt::layer())
        .with(otel_layer)
        .try_init()
        .ok();
    Ok(())
}

#[cfg(not(feature = "otel"))]
pub fn init_tracing(_cfg: &crate::config::AppConfig) -> anyhow::Result<()> { Ok(()) }
