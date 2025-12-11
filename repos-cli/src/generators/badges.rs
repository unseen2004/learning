use std::path::Path;
use walkdir::WalkDir;
use std::collections::HashSet;

pub fn generate_badges(path: &Path) -> String {
    let mut extensions = HashSet::new();

    for entry in WalkDir::new(path).into_iter().filter_map(|e| e.ok()) {
        if entry.file_type().is_file() {
            if let Some(ext) = entry.path().extension() {
                if let Some(ext_str) = ext.to_str() {
                    extensions.insert(ext_str.to_lowercase());
                }
            }
        }
    }

    let mut badges = String::new();

    if extensions.contains("rs") {
        badges.push_str("[![Rust](https://img.shields.io/badge/rust-%23000000.svg?style=for-the-badge&logo=rust&logoColor=white)](https://www.rust-lang.org/)\n");
    }
    if extensions.contains("py") {
        badges.push_str("[![Python](https://img.shields.io/badge/python-3670A0?style=for-the-badge&logo=python&logoColor=ffdd54)](https://www.python.org/)\n");
    }
    if extensions.contains("js") {
        badges.push_str("[![JavaScript](https://img.shields.io/badge/javascript-%23323330.svg?style=for-the-badge&logo=javascript&logoColor=%23F7DF1E)](https://developer.mozilla.org/en-US/docs/Web/JavaScript)\n");
    }
    if extensions.contains("ts") {
        badges.push_str("[![TypeScript](https://img.shields.io/badge/typescript-%23007ACC.svg?style=for-the-badge&logo=typescript&logoColor=white)](https://www.typescriptlang.org/)\n");
    }
    if extensions.contains("go") {
        badges.push_str("[![Go](https://img.shields.io/badge/go-%2300ADD8.svg?style=for-the-badge&logo=go&logoColor=white)](https://golang.org/)\n");
    }
    // Add more as needed

    badges
}
