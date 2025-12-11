pub mod badges;
pub mod dependencies;
pub mod docker;
pub mod changelog;
pub mod structure;

use std::path::Path;
use crate::config::Features;

pub fn generate_content(path: &Path, features: &Features) -> String {
    let mut content = String::new();

    if features.badges {
        content.push_str(&badges::generate_badges(path));
        content.push_str("\n");
    }

    if features.categorizer {
        content.push_str(&structure::generate_structure(path));
    }

    if features.dependencies {
        content.push_str(&dependencies::generate_dependencies(path));
    }

    if features.docker {
        content.push_str(&docker::generate_docker(path));
    }

    if features.changelog {
        content.push_str(&changelog::generate_changelog(path));
    }

    content
}
