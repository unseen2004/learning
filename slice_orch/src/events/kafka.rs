// Placeholder for Kafka integration using rdkafka

#[cfg(feature = "kafka")]
use rdkafka::{consumer::{StreamConsumer, Consumer}, ClientConfig, Message};
#[cfg(feature = "kafka")]
use crate::telemetry::{ingest, model::TelemetrySample, metrics};
#[cfg(feature = "kafka")]
use futures::StreamExt;

#[cfg(feature = "kafka")]
pub async fn spawn_kafka_consumer(brokers: &str, topic: &str) -> anyhow::Result<tokio::task::JoinHandle<()>> {
    let consumer: StreamConsumer = ClientConfig::new()
        .set("bootstrap.servers", brokers)
        .set("group.id", "slice-orchestrator")
        .set("enable.partition.eof", "false")
        .set("session.timeout.ms", "6000")
        .set("enable.auto.commit", "true")
        .create()?;
    consumer.subscribe(&[topic])?;
    tracing::info!(%brokers, %topic, "Kafka consumer subscribed");
    Ok(tokio::spawn(async move {
        let mut stream = consumer.stream();
        while let Some(result) = stream.next().await {
            match result {
                Ok(m) => {
                    if let Some(payload) = m.payload() {
                        if let Ok(txt) = std::str::from_utf8(payload) {
                            if let Ok(sample) = serde_json::from_str::<TelemetrySample>(txt) {
                                metrics::record_latency(sample.latency_ms);
                                ingest::ingest_sample(sample).await;
                            }
                        }
                    }
                }
                Err(e) => tracing::error!(error=%e, "kafka error"),
            }
        }
    }))
}

#[cfg(not(feature = "kafka"))]
pub async fn spawn_kafka_consumer(_brokers: &str, _topic: &str) -> anyhow::Result<tokio::task::JoinHandle<()>> { anyhow::bail!("kafka feature disabled") }
