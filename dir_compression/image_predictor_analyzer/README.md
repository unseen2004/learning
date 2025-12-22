# Image Predictor Analyzer

A tool for analyzing the entropy of TGA images using various predictive models (JPEG-LS predictors).

## Description

This program reads uncompressed 24-bit TGA images and calculates the entropy of the RGB color channels. It then applies 8 different predictive models (based on neighboring pixels) to generate residual images and calculates the entropy of these residuals. This helps determine which predictor is best suited for compressing a specific image.

## Predictors Implemented
1. **W (a)**: West neighbor
2. **N (b)**: North neighbor
3. **NW (c)**: North-West neighbor
4. **a + b - c**
5. **a + (b - c) / 2**
6. **b + (a - c) / 2**
7. **(a + b) / 2**: Average
8. **New**: JPEG-LS LOCO-I (Median Edge Detector)

## Usage

### Building

```bash
mkdir build
cd build
cmake ..
make
```

### Running

Analyze all TGA files in the `example` directory:
```bash
./image_predictor_analyzer
```

Analyze specific files:
```bash
./image_predictor_analyzer image1.tga image2.tga
```
