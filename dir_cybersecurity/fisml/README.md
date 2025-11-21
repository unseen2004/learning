# FISML (File Integrity & Secret Leakage Monitor)

Minimal Rust MVP for monitoring file changes and scanning for potential secret leaks.

## Features (MVP)
- Baseline scan of configured paths
- Detect new / modified files (hash diff)
- Optional secret scanning (regex + entropy heuristic)
- SQLite backed state and append-only event chain (hash-linked)
- Simple watch mode with Ctrl+C graceful exit

## Install / Build
```bash
cargo build --release
```

## Usage
```bash
# Create default config (fisml.toml) if missing
cargo run -- init-config

# Run baseline (with secret scanning)
cargo run -- baseline --secrets

# List recent events
cargo run -- list-events --limit 20

# Watch for changes (and secret scan)
cargo run -- watch --secrets

# Verify event hash chain integrity
cargo run -- verify-chain
```

Set RUST_LOG for more verbose tracing:
```bash
RUST_LOG=info cargo run -- watch
```

## Config
Automatically generated at first run (fisml.toml). Edit paths / ignore patterns / entropy threshold.


## Disclaimer
MVP quality. Not production hardened. Use at your own risk.
