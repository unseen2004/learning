# Repos CLI

A production-ready Rust CLI tool for automating README.md updates across your GitHub repositories.

Automatically generate and maintain README content with tech stack badges, dependency lists, changelogs, Docker commands, and project structure—all synced with your codebase.

## Features

**Secure Authentication**
- GitHub Personal Access Token (PAT) authentication
- Credentials stored in system keyring (encrypted, never in plain text)
- Supports Linux (Secret Service), macOS (Keychain), Windows (Credential Manager)

**Auto-Generated README Content**
- Tech Stack Badges: Shields.io badges based on file extensions
- Dependency Lists: Summarizes packages from `package.json`, `Cargo.toml`, `requirements.txt`
- Docker Commands: Auto-generates build/run commands if `Dockerfile` exists
- Changelogs: Recent git commit history
- Project Structure: Lists sub-projects in monorepos

**Recursive Updates**
- Automatically updates nested project READMEs
- Detects and processes sub-projects (any directory with `Cargo.toml`, `package.json`, etc.)
- Configurable per-directory with `.repos-cli.toml`

**Production-Ready**
- Secure credential storage (system keyring)
- Graceful error handling with structured logging
- Automated integration tests
- No crashes from malformed files or permission issues

## Installation

```bash
cargo install --path .
```

Or use a pre-built binary:
```bash
cargo build --release
cp target/release/repos-cli /usr/local/bin/
```

## Usage

Authenticate with GitHub:
```bash
repos-cli auth login
```

Track repositories:
```bash
repos-cli add /path/to/your/repo
```

Remove repositories from tracking:
```bash
repos-cli remove /path/to/your/repo
```

Show tracked repositories:
```bash
repos-cli tracked
```

Update READMEs:
```bash
repos-cli update
```

List your GitHub repositories:
```bash
repos-cli list
```

## Configuration

Global config is stored at `~/.config/repos-cli/config.toml`:
```toml
github_username = "your-username"
tracked_repos = ["/path/to/repo1", "/path/to/repo2"]
```

Local config (`.repos-cli.toml`) can be added to any repository:
```toml
auto_update = true

[features]
badges = true
emoji = true
changelog = true
dependencies = true
docker = true
categorizer = true
```

See `.repos-cli.toml.example` for a complete configuration file with detailed documentation and usage examples.

## How It Works

Generated content is injected between markers in your README:

```markdown
<!-- REPOS-CLI:START -->
(Auto-generated content)
<!-- REPOS-CLI:END -->
```

Content outside the markers is preserved.

## Security

- Credentials stored in OS-native encrypted keyring
- Config files contain only non-sensitive data
- Credentials never committed to version control

## License

MIT
