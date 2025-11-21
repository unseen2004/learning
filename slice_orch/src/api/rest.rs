use axum::{Router, routing::get};

pub fn router() -> Router {
    Router::new().route("/api/v1/ping", get(|| async { "pong" }))
}

