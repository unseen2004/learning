use std::path::Path;
use std::fs;
use anyhow::Result;
use crate::config::{LocalConfig, Features};
use crate::generators::generate_content;

pub fn update_readme(path: &Path) -> Result<()> {
    // Load local config if exists, otherwise use default
    let config_path = path.join(".repos-cli.toml");
    let features = if config_path.exists() {
        let content = fs::read_to_string(config_path)?;
        let config: LocalConfig = toml::from_str(&content)?;
        if !config.auto_update {
            return Ok(());
        }
        config.features
    } else {
        Features::default()
    };

    let generated_content = generate_content(path, &features);
    if generated_content.is_empty() {
        return Ok(());
    }

    let readme_path = path.join("README.md");
    let mut content = if readme_path.exists() {
        fs::read_to_string(&readme_path)?
    } else {
        String::new()
    };

    let start_marker = "<!-- REPOS-CLI:START -->";
    let end_marker = "<!-- REPOS-CLI:END -->";

    if let Some(start) = content.find(start_marker) {
        if let Some(end) = content.find(end_marker) {
            // Replace existing block
            let before = &content[..start];
            let after = &content[end + end_marker.len()..];
            content = format!("{}{}\n{}\n{}{}", before, start_marker, generated_content, end_marker, after);
        } else {
            // End marker missing, append
            content.push_str(&format!("\n{}\n{}\n{}\n", start_marker, generated_content, end_marker));
        }
    } else {
        // No markers, append
        content.push_str(&format!("\n{}\n{}\n{}\n", start_marker, generated_content, end_marker));
    }

    fs::write(readme_path, content)?;
    println!("Updated README.md for {:?}", path);

    // Recursive update for sub-projects
    for entry in walkdir::WalkDir::new(path).min_depth(1).max_depth(1).into_iter().filter_map(|e| e.ok()) {
        if entry.file_type().is_dir() {
            let dir_path = entry.path();
            // Check if it looks like a project
            if dir_path.join("Cargo.toml").exists() 
                || dir_path.join("package.json").exists() 
                || dir_path.join("requirements.txt").exists() 
                || dir_path.join("Makefile").exists() {
                
                // Skip hidden dirs and target/node_modules
                if let Some(name) = dir_path.file_name().and_then(|n| n.to_str()) {
                    if !name.starts_with('.') && name != "target" && name != "node_modules" {
                        if let Err(e) = update_readme(dir_path) {
                            tracing::error!("Failed to update README for {:?}: {}", dir_path, e);
                            eprintln!("Warning: Failed to update README for {:?}: {}", dir_path, e);
                        }
                    }
                }
            }
        }
    }

    Ok(())
}
