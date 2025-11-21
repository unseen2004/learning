use crate::types::{SliceSpec, SliceStatus};
use std::sync::atomic::{AtomicU32, Ordering};
use once_cell::sync::Lazy;

static TOTAL_BW: Lazy<AtomicU32> = Lazy::new(|| AtomicU32::new(0));
static USED_BW: Lazy<AtomicU32> = Lazy::new(|| AtomicU32::new(0));

pub fn init(total_mbps: u32) { TOTAL_BW.store(total_mbps, Ordering::Relaxed); USED_BW.store(0, Ordering::Relaxed); }

fn ensure_default_capacity() {
    // Provide a generous default capacity for tests / legacy code paths that did not call init.
    const DEFAULT_CAPACITY: u32 = 1_000_000; // 1 Tbps logical placeholder
    let current = TOTAL_BW.load(Ordering::Relaxed);
    if current == 0 {
        let _ = TOTAL_BW.compare_exchange(0, DEFAULT_CAPACITY, Ordering::SeqCst, Ordering::SeqCst);
    }
}

pub fn allocate(spec: &SliceSpec) -> SliceStatus {
    ensure_default_capacity();
    let needed = spec.bandwidth_mbps;
    loop {
        let used = USED_BW.load(Ordering::Relaxed);
        let total = TOTAL_BW.load(Ordering::Relaxed);
        if used + needed > total { return SliceStatus { id: spec.id.clone(), allocated: false, current_bandwidth_mbps: 0 }; }
        if USED_BW.compare_exchange(used, used + needed, Ordering::SeqCst, Ordering::SeqCst).is_ok() { break; }
    }
    crate::telemetry::metrics::record_alloc(spec.bandwidth_mbps);
    SliceStatus { id: spec.id.clone(), allocated: true, current_bandwidth_mbps: spec.bandwidth_mbps }
}

pub fn used_bandwidth() -> u32 { USED_BW.load(Ordering::Relaxed) }

#[doc(hidden)]
pub fn _reset_for_tests() { TOTAL_BW.store(0, Ordering::Relaxed); USED_BW.store(0, Ordering::Relaxed); }
