# Queue System

QR Code Queue Management System in Rust

## Features

- Generate unique QR codes
- Scan QR codes with camera (OpenCV)
- First person auto-removed after 20 seconds
- Re-scan shows position in queue
- CLI interface

## Installation

```bash
cargo build --release
```

## Usage

### Interactive Mode
```bash
cargo run -- run
```

### Generate QR Code
```bash
cargo run -- generate
```

### Scan QR Code
```bash
cargo run -- scan
```

### Check Status
```bash
cargo run -- status
```

## Dependencies

- qrcode
- opencv
- tokio
- clap
- crossterm
- chrono
- uuid
