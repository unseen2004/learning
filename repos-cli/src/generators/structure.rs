use std::path::Path;
use walkdir::WalkDir;

pub fn generate_structure(path: &Path) -> String {
    let mut structure = String::new();
    let mut projects = Vec::new();

    for entry in WalkDir::new(path).min_depth(1).max_depth(2).into_iter().filter_map(|e| e.ok()) {
        if entry.file_type().is_dir() {
            let dir_path = entry.path();
            // Check if it looks like a project
            if dir_path.join("Cargo.toml").exists() 
                || dir_path.join("package.json").exists() 
                || dir_path.join("requirements.txt").exists() 
                || dir_path.join("Makefile").exists() {
                
                if let Some(name) = dir_path.file_name().and_then(|n| n.to_str()) {
                    if !name.starts_with('.') && name != "target" && name != "node_modules" {
                        projects.push(name.to_string());
                    }
                }
            }
        }
    }

    if !projects.is_empty() {
        projects.sort();
        structure.push_str("## Sub-Projects\n\n");
        for project in projects {
            structure.push_str(&format!("- **{}**\n", project));
        }
        structure.push_str("\n");
    }

    structure
}
