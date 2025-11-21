use slice_orchestrator::controller::allocator;
use slice_orchestrator::types::{SliceId, SliceSpec};
use serial_test::serial;

#[test]
#[serial]
fn capacity_enforced() {
    allocator::_reset_for_tests();
    allocator::init(150); // capacity 150 Mbps
    let s1 = SliceSpec { id: SliceId("cap1".into()), latency_sla_ms: 10, bandwidth_mbps: 100, isolation: false };
    let s2 = SliceSpec { id: SliceId("cap2".into()), latency_sla_ms: 10, bandwidth_mbps: 100, isolation: false };
    let st1 = allocator::allocate(&s1); assert!(st1.allocated);
    let st2 = allocator::allocate(&s2); assert!(!st2.allocated, "second allocation should fail due to capacity");
}
