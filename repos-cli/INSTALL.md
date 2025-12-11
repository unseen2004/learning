# Installation Requirements

## Linux Dependencies

On Linux, the keyring functionality requires `libsecret`:

```bash
# Debian/Ubuntu
sudo apt install libsecret-tools libsecret-1-dev

# Fedora/RHEL
sudo dnf install libsecret libsecret-devel

# Arch
sudo pacman -S libsecret
```

## Install the CLI

```bash
cargo install --path .
```
