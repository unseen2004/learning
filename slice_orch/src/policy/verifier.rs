use super::dsl::parse_rule;

pub fn verify_policies(policies: &[String]) -> bool {
    if policies.is_empty() { return false; }
    policies.iter().all(|p| !p.trim().is_empty() && parse_rule(p).is_ok())
}
