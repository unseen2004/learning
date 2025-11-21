use crate::types::{SliceId, SliceStatus, SliceSpec};
use dashmap::DashMap;
use once_cell::sync::Lazy;

static SLICES: Lazy<DashMap<String, SliceStatus>> = Lazy::new(|| DashMap::new());

pub fn put(spec: &SliceSpec, status: SliceStatus) { SLICES.insert(spec.id.0.clone(), status); }
pub fn get(id: &SliceId) -> Option<SliceStatus> { SLICES.get(&id.0).map(|v| v.clone()) }

