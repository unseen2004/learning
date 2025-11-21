use super::model::AllocationProblem;
pub fn feasible(p: &AllocationProblem) -> bool { p.demand <= p.capacity }

