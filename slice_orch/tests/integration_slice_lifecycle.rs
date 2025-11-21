use slice_orchestrator::{types::{SliceSpec, SliceId}, controller::{allocator, admission}};
use serial_test::serial;

#[test]
#[serial]
fn allocate_basic_slice() {
    let spec = SliceSpec { id: SliceId("s1".into()), latency_sla_ms: 50, bandwidth_mbps: 100, isolation: true };
    assert!(admission::admit(&spec));
    let status = allocator::allocate(&spec);
    assert!(status.allocated);
    assert_eq!(status.current_bandwidth_mbps, 100);
}
