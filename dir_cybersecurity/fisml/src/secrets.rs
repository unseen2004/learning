use regex::Regex;
use crate::model::SecretFinding;
use crate::hashing::hash_bytes;

// Simple entropy calculation (Shannon)
pub fn shannon_entropy(data: &str) -> f64 {
    let bytes = data.as_bytes();
    if bytes.is_empty() { return 0.0; }
    let mut counts = [0usize; 256];
    for b in bytes { counts[*b as usize] += 1; }
    let len = bytes.len() as f64;
    let mut entropy = 0.0;
    for c in counts.iter().copied().filter(|c| *c > 0) {
        let p = c as f64 / len;
        entropy -= p * p.log2();
    }
    entropy
}

pub struct SecretScanner {
    patterns: Vec<(String, Regex)>,
    entropy_threshold: f64,
}

impl SecretScanner {
    pub fn new(entropy_threshold: f64) -> Self {
        let mut patterns = Vec::new();
        // Common tokens (simplified)
        let defs: Vec<(&str, &str)> = vec![
            ("AWS_ACCESS_KEY", r"AKIA[0-9A-Z]{16}"),
            ("AWS_SECRET_KEY", r"(?i)aws(.{0,20})?(secret|access).{0,3}[0-9a-z/+]{40}"),
            ("GCP_API_KEY", r"AIza[0-9A-Za-z\-_]{35}"),
            ("JWT", r"eyJ[a-zA-Z0-9_\-]+=*\.[a-zA-Z0-9_\-]+=*\.[a-zA-Z0-9_\-\.=]*"),
            ("URL_BASIC_AUTH", r"https?://[^/]+:[^/]+@"),
            ("PRIVATE_KEY", r"-----BEGIN( RSA)? PRIVATE KEY-----"),
        ];
        for (name, pat) in defs { if let Ok(r) = Regex::new(pat) { patterns.push((name.to_string(), r)); } }
        Self { patterns, entropy_threshold }
    }

    pub fn scan(&self, content: &str) -> Vec<SecretFinding> {
        let mut findings = Vec::new();
        for (kind, re) in &self.patterns {
            for m in re.find_iter(content) {
                let span = (m.start(), m.end());
                let preview = &content[m.start()..m.end()].as_bytes();
                findings.push(SecretFinding {
                    kind: kind.clone(),
                    span,
                    entropy: shannon_entropy(m.as_str()),
                    preview_hash: hash_bytes(preview),
                });
            }
        }
        // High entropy tokens (rough splitting by non-word)
        for token in content.split(|c: char| !c.is_ascii_alphanumeric()) {
            if token.len() >= 20 {
                let e = shannon_entropy(token);
                if e >= self.entropy_threshold {
                    let hash = hash_bytes(token.as_bytes());
                    if let Some(pos) = content.find(token) { // first occurrence
                        findings.push(SecretFinding { kind: "HIGH_ENTROPY".into(), span: (pos, pos + token.len()), entropy: e, preview_hash: hash });
                    }
                }
            }
        }
        findings
    }
}
