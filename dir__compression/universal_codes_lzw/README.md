# LZW Universal Coding

An implementation of the LZW compression algorithm using various universal codes for output stream encoding.

## Description

This tool compresses files using the Lempel-Ziv-Welch (LZW) algorithm. Instead of fixed-length codes, it allows encoding the LZW dictionary indices using different universal variable-length codes:
- Elias Gamma
- Elias Delta
- Elias Omega
- Fibonacci Coding

## Usage

### Building

```bash
mkdir build
cd build
cmake ..
make
```

### Running

**Encoding:**
```bash
./universal_codes_lzw encode <input_file> <output_file> [coding_type]
```
Supported `coding_type`: `omega` (default), `gamma`, `delta`, `fibonacci`.

**Decoding:**
```bash
./universal_codes_lzw decode <input_file> <output_file> [coding_type]
```
*Note: You must use the same `coding_type` for decoding as used for encoding.*

### Example

```bash
./universal_codes_lzw encode data.bin compressed.lzw fibonacci
./universal_codes_lzw decode compressed.lzw recovered.bin fibonacci
```
