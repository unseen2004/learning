use regex::Regex;

pub fn parse_rule(s: &str) -> Result<(), String> {
    let trimmed = s.trim();
    if !trimmed.starts_with("rule ") { return Err("missing 'rule' keyword".into()); }
    // Extract name before first '{'
    let parts: Vec<&str> = trimmed.splitn(2, '{').collect();
    if parts.len() < 2 { return Err("missing '{'".into()); }
    let header = parts[0].trim(); // e.g. "rule test"
    let mut header_tokens = header.split_whitespace();
    let keyword = header_tokens.next().unwrap_or("");
    if keyword != "rule" { return Err("invalid keyword".into()); }
    let name = header_tokens.next().ok_or_else(|| "missing rule name".to_string())?;
    if header_tokens.next().is_some() { return Err("unexpected tokens after rule name".into()); }
    let name_re = Regex::new(r"^[A-Za-z_][A-Za-z0-9_]*$").unwrap();
    if !name_re.is_match(name) { return Err("invalid rule name".into()); }
    // ensure braces balanced and ends with '}'
    if !trimmed.ends_with('}') { return Err("missing closing '}'".into()); }
    let open_count = trimmed.chars().filter(|c| *c == '{').count();
    let close_count = trimmed.chars().filter(|c| *c == '}').count();
    if open_count != close_count { return Err("unbalanced braces".into()); }
    Ok(())
}
