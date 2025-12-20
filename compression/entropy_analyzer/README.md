# Entropy Analyzer

A C++ utility for analyzing the entropy and conditional entropy of files.

## Description

This tool reads a binary file and calculates:
- Symbol frequencies (byte distribution).
- **H(Y)**: The first-order entropy (Shannon entropy).
- **H(Y|X)**: The conditional entropy, where X is the preceding byte.
- Information Gain: The difference between H(Y) and H(Y|X), indicating how much the previous byte predicts the current one.

## Usage

### Building

```bash
mkdir build
cd build
cmake ..
make
```

### Running

```bash
./entropy_analyzer <file_path>
```

### Example Output

```text
=== ENTROPY ANALYSIS ===

File size: 102400 bytes
Unique symbols: 256

SYMBOL STATISTICS:
Byte   Count       Probability 
------------------------------
'a'    5000        0.048828    
...

=== ENTROPY MEASURES ===
H(Y) - Total Entropy:              7.999999 bits
H(Y|X) - Conditional Entropy:      7.500000 bits
H(Y) - H(Y|X) - Information Gain:  0.499999 bits
```
