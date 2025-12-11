use std::path::Path;

pub fn generate_docker(path: &Path) -> String {
    let dockerfile_path = path.join("Dockerfile");
    if dockerfile_path.exists() {
        let repo_name = path.file_name()
            .and_then(|n| n.to_str())
            .unwrap_or("app")
            .to_lowercase();

        return format!(
            "## Docker\n\nBuild and run with Docker:\n\n```bash\ndocker build -t {} .\ndocker run -it {}\n```\n\n",
            repo_name, repo_name
        );
    }
    String::new()
}
