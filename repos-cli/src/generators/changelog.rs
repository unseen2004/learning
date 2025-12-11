use std::path::Path;
use std::process::Command;

pub fn generate_changelog(path: &Path) -> String {
    let output = Command::new("git")
        .args(&["log", "-n", "5", "--pretty=format:- %s (%h)"])
        .current_dir(path)
        .output();

    match output {
        Ok(output) => {
            if output.status.success() {
                let log = String::from_utf8_lossy(&output.stdout);
                if !log.trim().is_empty() {
                    return format!("## Recent Updates\n\n{}\n\n", log);
                }
            }
        }
        Err(_) => {}
    }
    String::new()
}
