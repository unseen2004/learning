use std::path::Path;
use std::fs;
use serde::Deserialize;
use std::collections::HashMap;

#[derive(Deserialize)]
struct PackageJson {
    dependencies: Option<HashMap<String, String>>,
    #[serde(rename = "devDependencies")]
    dev_dependencies: Option<HashMap<String, String>>,
}

#[derive(Deserialize)]
struct CargoToml {
    dependencies: Option<HashMap<String, toml::Value>>,
}

pub fn generate_dependencies(path: &Path) -> String {
    let mut deps_summary = String::new();

    // Check package.json
    let package_json_path = path.join("package.json");
    if package_json_path.exists() {
        if let Ok(content) = fs::read_to_string(package_json_path) {
            if let Ok(json) = serde_json::from_str::<PackageJson>(&content) {
                let mut deps = Vec::new();
                if let Some(d) = json.dependencies {
                    deps.extend(d.keys().cloned());
                }
                if let Some(d) = json.dev_dependencies {
                    deps.extend(d.keys().cloned());
                }
                
                if !deps.is_empty() {
                    deps.sort();
                    deps_summary.push_str("**Node.js Dependencies:** ");
                    deps_summary.push_str(&deps.iter().take(10).cloned().collect::<Vec<_>>().join(", "));
                    if deps.len() > 10 {
                        deps_summary.push_str(", and more...");
                    }
                    deps_summary.push_str("\n\n");
                }
            }
        }
    }

    // Check Cargo.toml
    let cargo_toml_path = path.join("Cargo.toml");
    if cargo_toml_path.exists() {
        if let Ok(content) = fs::read_to_string(cargo_toml_path) {
            if let Ok(toml) = toml::from_str::<CargoToml>(&content) {
                if let Some(d) = toml.dependencies {
                    let mut deps: Vec<String> = d.keys().cloned().collect();
                    deps.sort();
                    
                    if !deps.is_empty() {
                        deps_summary.push_str("**Rust Dependencies:** ");
                        deps_summary.push_str(&deps.iter().take(10).cloned().collect::<Vec<_>>().join(", "));
                        if deps.len() > 10 {
                            deps_summary.push_str(", and more...");
                        }
                        deps_summary.push_str("\n\n");
                    }
                }
            }
        }
    }

    // Check requirements.txt
    let req_path = path.join("requirements.txt");
    if req_path.exists() {
        if let Ok(content) = fs::read_to_string(req_path) {
            let deps: Vec<&str> = content.lines()
                .filter(|l| !l.trim().is_empty() && !l.starts_with('#'))
                .map(|l| l.split("==").next().unwrap_or(l).trim())
                .collect();
            
            if !deps.is_empty() {
                deps_summary.push_str("**Python Dependencies:** ");
                deps_summary.push_str(&deps.iter().take(10).cloned().collect::<Vec<_>>().join(", "));
                if deps.len() > 10 {
                    deps_summary.push_str(", and more...");
                }
                deps_summary.push_str("\n\n");
            }
        }
    }

    deps_summary
}
