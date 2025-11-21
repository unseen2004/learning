use slice_orchestrator::telemetry::ingest::ingest_batch;

#[tokio::test]
async fn telemetry_batch_ingest_handles_mixed_lines() {
    let data = b"{\"slice_id\":\"s1\",\"latency_ms\":42}\ninvalid_json\n{\"slice_id\":\"s1\",\"latency_ms\":55}\n";
    ingest_batch(data).await; // Should not panic
}

