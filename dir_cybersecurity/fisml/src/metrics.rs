use prometheus::{IntCounterVec, IntCounter, Encoder, TextEncoder, register_int_counter_vec, register_int_counter};
use once_cell::sync::Lazy;
use std::net::SocketAddr;
use axum::{routing::get, Router, response::IntoResponse};
use axum::http::StatusCode;
use axum::serve;
use tokio::net::TcpListener;
use crate::model::EventKind;

static EVENTS: Lazy<IntCounterVec> = Lazy::new(|| {
    register_int_counter_vec!("fisml_events_total", "Number of events by kind", &["kind"]).unwrap()
});
static SECRETS: Lazy<IntCounter> = Lazy::new(|| {
    register_int_counter!("fisml_secrets_total", "Total secret findings").unwrap()
});

pub fn inc_events(kind: EventKind) {
    let label = match kind { EventKind::Baseline => "baseline", EventKind::New => "new", EventKind::Modified => "modified", EventKind::Deleted => "deleted" };
    EVENTS.with_label_values(&[label]).inc();
}

pub fn inc_secrets(n: u64) { SECRETS.inc_by(n); }

async fn metrics_handler() -> impl IntoResponse {
    let mut buf = Vec::new();
    let encoder = TextEncoder::new();
    let mf = prometheus::gather();
    if encoder.encode(&mf, &mut buf).is_ok() {
        (StatusCode::OK, String::from_utf8_lossy(&buf).to_string())
    } else {
        (StatusCode::INTERNAL_SERVER_ERROR, "encode error".to_string())
    }
}

pub fn spawn_metrics(addr: SocketAddr) {
    std::thread::spawn(move || {
        let rt = tokio::runtime::Runtime::new().expect("rt");
        rt.block_on(async move {
            let app = Router::new().route("/metrics", get(metrics_handler));
            tracing::info!(address=%addr, "Starting metrics server");
            match TcpListener::bind(addr).await {
                Ok(listener) => {
                    if let Err(e) = serve(listener, app.into_make_service()).await { eprintln!("Metrics server error: {e}"); }
                }
                Err(e) => eprintln!("Metrics bind error: {e}")
            }
        });
    });
}
