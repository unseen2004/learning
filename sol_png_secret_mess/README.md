# Solidity PNG Secret Messenger

A command-line tool written in Solidity (using Foundry) to hide secret messages in PNG files.

## Prerequisites

- **Foundry**: You need to have Foundry installed.
  ```bash
  curl -L https://foundry.paradigm.xyz | bash
  foundryup
  ```

## Setup

1. Clone the repository (or navigate to it).
2. Install dependencies:
   ```bash
   forge install
   ```

## Usage

This tool uses `forge script` to run Solidity code that interacts with your filesystem.

### 1. Encode a Message
Hides a message in a PNG file by adding a custom chunk.

```bash
MODE=encode \
FILE=path/to/image.png \
CHUNK_TYPE=secr \
MESSAGE="This is a secret" \
OUTPUT=encoded.png \
forge script script/PngTool.s.sol
```

- `CHUNK_TYPE`: A 4-letter code for your chunk (e.g., `secr`, `ruSt`, `hide`).
- `OUTPUT`: (Optional) Output file path. Defaults to `input.png.out.png`.

### 2. Decode a Message
Reads a message from a specific chunk type.

```bash
MODE=decode \
FILE=encoded.png \
CHUNK_TYPE=secr \
forge script script/PngTool.s.sol
```

### 3. Remove a Message
Removes all chunks of a specific type.

```bash
MODE=remove \
FILE=encoded.png \
CHUNK_TYPE=secr \
OUTPUT=clean.png \
forge script script/PngTool.s.sol
```

### 4. Print Chunks
Lists all chunks in the PNG file to verify structure.

```bash
MODE=print \
FILE=image.png \
forge script script/PngTool.s.sol
```

## Integration into a Git Monorepo

If you want to add this project to a larger monorepo, follow these steps:

### Option 1: As a Subdirectory (Recommended)

1.  **Copy the folder**: Move this entire directory into your monorepo (e.g., `packages/png-tool`).
2.  **Dependencies**: Ensure the `lib` folder (containing `forge-std`) is included or re-installed.
    *   If your monorepo uses git submodules, run `forge install` inside the `packages/png-tool` directory.
3.  **Running**: You should run `forge` commands from within the `packages/png-tool` directory.
    ```bash
    cd packages/png-tool
    forge test
    ```

### Option 2: Shared Foundry Config

If your monorepo is a large Foundry project with multiple contracts:

1.  **Source Files**: Copy `src/PngLib.sol` to your shared `src` or `lib` directory.
2.  **Scripts**: Copy `script/PngTool.s.sol` to your `script` directory.
3.  **Config**: Update your root `foundry.toml` to allow file system access:
    ```toml
    [profile.default]
    fs_permissions = [{ access = "read-write", path = "./" }]
    ```
    *Note: Be careful with `read-write` permissions on `./` in a large repo. You might want to restrict it to specific data directories.*

### Troubleshooting

- **File Access Errors**: If you see `vm.readFileBinary: ... not allowed`, check `foundry.toml`.
  ```toml
  fs_permissions = [{ access = "read-write", path = "./" }]
  ```
  This setting allows the script to read/write files in the project directory.
