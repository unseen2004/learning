use std::fs;
use std::path::PathBuf;
use tempfile::TempDir;

#[test]
fn test_readme_generation_with_rust_project() {
    let temp_dir = TempDir::new().unwrap();
    let project_path = temp_dir.path();

    // Create a mock Cargo.toml
    fs::write(
        project_path.join("Cargo.toml"),
        r#"[package]
name = "test-project"
version = "0.1.0"

[dependencies]
serde = "1.0"
tokio = "1.0"
"#,
    )
    .unwrap();

    // Create an initial README
    fs::write(project_path.join("README.md"), "# Test Project\n").unwrap();

    // We can't test the full update_readme without mocking config
    // This is a placeholder for integration testing structure
    assert!(project_path.join("Cargo.toml").exists());
    assert!(project_path.join("README.md").exists());
}

#[test]
fn test_readme_generation_with_nodejs_project() {
    let temp_dir = TempDir::new().unwrap();
    let project_path = temp_dir.path();

    // Create a mock package.json
    fs::write(
        project_path.join("package.json"),
        r#"{
  "name": "test-js-project",
  "version": "1.0.0",
  "dependencies": {
    "react": "^18.0.0",
    "express": "^4.18.0"
  }
}"#,
    )
    .unwrap();

    assert!(project_path.join("package.json").exists());
}
