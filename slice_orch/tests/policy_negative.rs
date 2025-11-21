use slice_orchestrator::policy::{dsl, verifier};

#[test]
fn invalid_rule_missing_keyword() {
    assert!(dsl::parse_rule("bad test { }").is_err());
}

#[test]
fn invalid_rule_bad_name() {
    assert!(dsl::parse_rule("rule 123bad { }").is_err());
}

#[test]
fn verify_policies_rejects_invalid() {
    let policies = vec!["rule good { }".into(), "bad rule".into()];
    assert!(!verifier::verify_policies(&policies));
}
use slice_orchestrator::controller::allocator;
use slice_orchestrator::types::{SliceSpec, SliceId};

#[test]
fn capacity_enforced() {
    allocator::_reset_for_tests();
    allocator::init(150); // capacity 150 Mbps
    let s1 = SliceSpec { id: SliceId("cap1".into()), latency_sla_ms: 10, bandwidth_mbps: 100, isolation: false };
    let s2 = SliceSpec { id: SliceId("cap2".into()), latency_sla_ms: 10, bandwidth_mbps: 100, isolation: false };
    let st1 = allocator::allocate(&s1); assert!(st1.allocated);
    let st2 = allocator::allocate(&s2); assert!(!st2.allocated, "second allocation should fail due to capacity");
}

