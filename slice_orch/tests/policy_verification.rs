use slice_orchestrator::policy::{dsl, verifier};

#[test]
fn verify_example_policies() {
    assert!(dsl::parse_rule("rule test { }").is_ok());
    let policies = vec![
        "rule alpha { }".into(),
        "rule beta123 { }".into(),
        "rule _internal { }".into(),
    ];
    assert!(verifier::verify_policies(&policies));
    // Negative sanity
    let bad = vec!["rule 123bad { }".into(), "badformat".into()];
    assert!(!verifier::verify_policies(&bad));
}
