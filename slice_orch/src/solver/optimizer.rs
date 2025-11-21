use super::model::AllocationProblem;
pub fn optimize(p: &AllocationProblem) -> u32 { p.capacity.min(p.demand) }

