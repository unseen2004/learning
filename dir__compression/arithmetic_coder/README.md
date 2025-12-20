# Arithmetic Coder

An implementation of adaptive arithmetic coding in C++.

## Description

This tool provides lossless data compression using arithmetic coding with an adaptive frequency model (Binary Indexed Tree / Fenwick Tree).

## Usage

### Building

```bash
mkdir build
cd build
cmake ..
make
```

### Running

**Compression:**
```bash
./arithmetic_coder -c <input_file> <output_file>
```

**Decompression:**
```bash
./arithmetic_coder -d <compressed_file> <output_file>
```

### Statistics

The program prints compression statistics including input/output sizes, model entropy, average code length, and compression ratio.
