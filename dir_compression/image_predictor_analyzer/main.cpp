#include <iostream>
#include <fstream>
#include <vector>
#include <cstdint>
#include <map>
#include <cmath>
#include <string>
#include <algorithm>
#include <iomanip>
#include <filesystem>

#pragma pack(push, 1)
struct TGAHeader {
    uint8_t  idlength;
    uint8_t  colormaptype;
    uint8_t  datatypecode;
    uint16_t colormaporigin;
    uint16_t colormaplength;
    uint8_t  colormapdepth;
    uint16_t x_origin;
    uint16_t y_origin;
    uint16_t width;
    uint16_t height;
    uint8_t  bitsperpixel;
    uint8_t  imagedescriptor;
};
#pragma pack(pop)

struct TGAImage {
    uint16_t width;
    uint16_t height;
    std::vector<uint8_t> r_channel;
    std::vector<uint8_t> g_channel;
    std::vector<uint8_t> b_channel;
};

enum class PredictorType {
    None,       // P1: a (Left)
    Left,       // P1: a
    Up,         // P2: b
    UpLeft,     // P3: c
    P4,         // a + b - c
    P5,         // a + (b - c) / 2
    P6,         // b + (a - c) / 2
    Average,    // P7: (a + b) / 2
    New         // P8: LOCO-I / JPEG-LS median edge detector
};

const std::vector<std::pair<PredictorType, std::string>> PREDICTORS = {
    {PredictorType::Left, "W (a)"},
    {PredictorType::Up, "N (b)"},
    {PredictorType::UpLeft, "NW (c)"},
    {PredictorType::P4, "a+b-c"},
    {PredictorType::P5, "a+(b-c)/2"},
    {PredictorType::P6, "b+(a-c)/2"},
    {PredictorType::Average, "(a+b)/2"},
    {PredictorType::New, "New (JPEG-LS)"}
};

bool load_tga(const std::string& file_path, TGAImage& image) {
    std::ifstream file(file_path, std::ios::binary);
    if (!file) {
        std::cerr << "Error: Cannot open file " << file_path << std::endl;
        return false;
    }

    TGAHeader header;
    file.read(reinterpret_cast<char*>(&header), sizeof(TGAHeader));
    if (!file) return false;

    // Only support uncompressed RGB (type 2) and 24-bit
    if (header.datatypecode != 2 || header.bitsperpixel != 24) {
        std::cerr << "Error: Unsupported TGA format. Only uncompressed 24-bit RGB is supported." << std::endl;
        return false;
    }

    image.width = header.width;
    image.height = header.height;

    // Skip ID field if present
    file.seekg(header.idlength, std::ios::cur);

    size_t pixel_count = image.width * image.height;
    std::vector<uint8_t> raw_data(pixel_count * 3);
    file.read(reinterpret_cast<char*>(raw_data.data()), raw_data.size());
    if (!file) {
        std::cerr << "Error: Could not read image data." << std::endl;
        return false;
    }

    image.b_channel.resize(pixel_count);
    image.g_channel.resize(pixel_count);
    image.r_channel.resize(pixel_count);

    // TGA stores BGR
    for (size_t i = 0; i < pixel_count; ++i) {
        image.b_channel[i] = raw_data[i * 3 + 0];
        image.g_channel[i] = raw_data[i * 3 + 1];
        image.r_channel[i] = raw_data[i * 3 + 2];
    }

    return true;
}

double calculate_entropy(const std::vector<uint8_t>& data) {
    if (data.empty()) return 0.0;
    
    std::map<uint8_t, uint32_t> counts;
    for (uint8_t byte : data) {
        counts[byte]++;
    }

    double entropy = 0.0;
    double total = static_cast<double>(data.size());
    
    for (const auto& [val, count] : counts) {
        double p = static_cast<double>(count) / total;
        entropy -= p * std::log2(p);
    }
    return entropy;
}

uint8_t get_pixel(const std::vector<uint8_t>& channel, int x, int y, int width, int height) {
    if (x < 0 || y < 0 || x >= width || y >= height) {
        return 0;
    }
    return channel[y * width + x];
}

std::vector<uint8_t> apply_predictor(const std::vector<uint8_t>& channel, int width, int height, PredictorType type) {
    std::vector<uint8_t> residuals(channel.size());
    
    for (int y = 0; y < height; ++y) {
        for (int x = 0; x < width; ++x) {
            uint8_t a = get_pixel(channel, x - 1, y, width, height);
            uint8_t b = get_pixel(channel, x, y - 1, width, height);
            uint8_t c = get_pixel(channel, x - 1, y - 1, width, height);
            uint8_t x_val = get_pixel(channel, x, y, width, height);
            
            int prediction = 0;
            switch (type) {
                case PredictorType::Left:    prediction = a; break;
                case PredictorType::Up:      prediction = b; break;
                case PredictorType::UpLeft:  prediction = c; break;
                case PredictorType::P4:      prediction = a + b - c; break;
                case PredictorType::P5:      prediction = a + (b - c) / 2; break;
                case PredictorType::P6:      prediction = b + (a - c) / 2; break;
                case PredictorType::Average: prediction = (a + b) / 2; break;
                case PredictorType::New:
                    if (c >= std::max(a, b)) prediction = std::min(a, b);
                    else if (c <= std::min(a, b)) prediction = std::max(a, b);
                    else prediction = a + b - c;
                    break;
                default: prediction = 0; break;
            }
            
            residuals[y * width + x] = static_cast<uint8_t>(x_val - prediction);
        }
    }
    return residuals;
}

void analyze_image(const std::string& path) {
    TGAImage image;
    if (!load_tga(path, image)) return;

    std::cout << "\n=======================================================\n";
    std::cout << "Analysis of: " << path << " (" << image.width << "x" << image.height << ")\n";
    std::cout << "=======================================================\n";

    double ent_r = calculate_entropy(image.r_channel);
    double ent_g = calculate_entropy(image.g_channel);
    double ent_b = calculate_entropy(image.b_channel);
    
    // Total raw entropy
    std::vector<uint8_t> all_raw;
    all_raw.reserve(image.r_channel.size() * 3);
    all_raw.insert(all_raw.end(), image.r_channel.begin(), image.r_channel.end());
    all_raw.insert(all_raw.end(), image.g_channel.begin(), image.g_channel.end());
    all_raw.insert(all_raw.end(), image.b_channel.begin(), image.b_channel.end());
    double ent_total = calculate_entropy(all_raw);

    std::cout << std::fixed << std::setprecision(4);
    std::cout << "Original Entropy:\n";
    std::cout << "R: " << ent_r << " | G: " << ent_g << " | B: " << ent_b << " | Combined: " << ent_total << "\n\n";
    
    std::cout << std::setw(20) << "Predictor" 
              << std::setw(10) << "R" 
              << std::setw(10) << "G" 
              << std::setw(10) << "B" 
              << std::setw(10) << "Total" << "\n";
    std::cout << std::string(60, '-') << "\n";

    double min_total = ent_total;
    std::string best_predictor = "None";

    for (const auto& [type, name] : PREDICTORS) {
        auto res_r = apply_predictor(image.r_channel, image.width, image.height, type);
        auto res_g = apply_predictor(image.g_channel, image.width, image.height, type);
        auto res_b = apply_predictor(image.b_channel, image.width, image.height, type);

        double e_r = calculate_entropy(res_r);
        double e_g = calculate_entropy(res_g);
        double e_b = calculate_entropy(res_b);

        std::vector<uint8_t> all_res;
        all_res.reserve(res_r.size() * 3);
        all_res.insert(all_res.end(), res_r.begin(), res_r.end());
        all_res.insert(all_res.end(), res_g.begin(), res_g.end());
        all_res.insert(all_res.end(), res_b.begin(), res_b.end());
        double e_total = calculate_entropy(all_res);

        std::cout << std::setw(20) << name
                  << std::setw(10) << e_r
                  << std::setw(10) << e_g
                  << std::setw(10) << e_b
                  << std::setw(10) << e_total << "\n";

        if (e_total < min_total) {
            min_total = e_total;
            best_predictor = name;
        }
    }

    std::cout << "\nBest Predictor (Combined): " << best_predictor << " (" << min_total << ")\n";
}

int main(int argc, char* argv[]) {
    std::vector<std::string> files;
    
    if (argc > 1) {
        for (int i = 1; i < argc; ++i) {
            files.push_back(argv[i]);
        }
    } else {
        // Default relative path if no args provided, assuming typical structure
        if (std::filesystem::exists("example")) {
            for (const auto& entry : std::filesystem::directory_iterator("example")) {
                if (entry.path().extension() == ".tga") {
                    files.push_back(entry.path().string());
                }
            }
        }
    }
    
    std::sort(files.begin(), files.end());

    if (files.empty()) {
        std::cerr << "No TGA files found in 'example/' or provided as arguments.\n";
        return 1;
    }

    for (const auto& file : files) {
        analyze_image(file);
    }

    return 0;
}
