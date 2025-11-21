use crate::types::SliceSpec;

pub fn admit(spec: &SliceSpec) -> bool {
    // placeholder policy: admit if bandwidth <= 1000 Mbps
    spec.bandwidth_mbps <= 1000
}

