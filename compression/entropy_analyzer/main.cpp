#include <iostream>
#include <fstream>
#include <vector>
#include <cmath>
#include <map>
#include <iomanip>
#include <algorithm>
#include <string>

struct AnalysisResult {
    std::vector<uint64_t> symbol_counts;
    std::vector<std::vector<uint64_t>> transition_counts;
    uint64_t total_bytes;
};

std::vector<uint8_t> read_file(const std::string& filepath) {
    std::ifstream file(filepath, std::ios::binary | std::ios::ate);
    if (!file) {
        throw std::runtime_error("Failed to open file: " + filepath);
    }
    std::streamsize size = file.tellg();
    if (size <= 0) {
        return {};
    }
    file.seekg(0, std::ios::beg);
    std::vector<uint8_t> buffer(size);
    if (!file.read(reinterpret_cast<char*>(buffer.data()), size)) {
        throw std::runtime_error("Failed to read file: " + filepath);
    }
    return buffer;
}

AnalysisResult analyze_data(const std::vector<uint8_t>& data) {
    AnalysisResult result;
    result.symbol_counts.resize(256, 0);
    result.transition_counts.resize(256, std::vector<uint64_t>(256, 0));
    result.total_bytes = data.size();

    if (data.empty()) return result;

    uint8_t prev = 0; 
    bool first = true;

    for (uint8_t byte : data) {
        result.symbol_counts[byte]++;
        if (!first) {
            result.transition_counts[prev][byte]++;
        }
        prev = byte;
        first = false;
    }
    
    // For consistency with the Rust version which seemed to count transitions for all bytes 
    // but handled the first byte's predecessor as 0 or simply started tracking transitions.
    // The Rust code: `let mut prev = 0u8; for &byte in data { ... transition_freq ... prev = byte; }`
    // This implies the first byte has a predecessor of 0.
    // Let's match that logic exactly.
    
    std::fill(result.symbol_counts.begin(), result.symbol_counts.end(), 0);
    for(auto& v : result.transition_counts) std::fill(v.begin(), v.end(), 0);
    
    prev = 0;
    for (uint8_t byte : data) {
        result.symbol_counts[byte]++;
        result.transition_counts[prev][byte]++;
        prev = byte;
    }

    return result;
}

int main(int argc, char* argv[]) {
    if (argc != 2) {
        std::cerr << "Usage: " << argv[0] << " <file>" << std::endl;
        return 1;
    }

    try {
        std::string filepath = argv[1];
        auto data = read_file(filepath);

        if (data.empty()) {
            std::cerr << "File is empty" << std::endl;
            return 1;
        }

        auto result = analyze_data(data);
        double total = static_cast<double>(result.total_bytes);

        // Calculate H(Y)
        double h_y = 0.0;
        for (uint64_t count : result.symbol_counts) {
            if (count > 0) {
                double p = static_cast<double>(count) / total;
                h_y -= p * std::log2(p);
            }
        }

        // Calculate H(Y|X)
        double h_y_given_x = 0.0;
        for (int x = 0; x < 256; ++x) {
            uint64_t count_x = result.symbol_counts[x]; // Rust logic used symbol_freq count of prev symbol
            // Rust: let prev_count = *symbol_freq.get(prev).unwrap_or(&1) as f64;
            // The Rust code iterates over transitions (prev, next_map).
            // prev is the index in transition_counts.
            // We need count of `prev` to normalize conditional probs.
            
            // Wait, in the Rust code:
            // `transition_freq` key is `prev`.
            // `symbol_freq` is used to get count of `prev`.
            // In my loop above: `result.transition_counts[prev][byte]++`
            // So `result.symbol_counts[prev]` is indeed the count of `prev` occurrences as a predecessor?
            // Not exactly. `symbol_counts` counts occurrences of the symbol anywhere.
            // But since we iterate `data`, `prev` takes every value in `data` (plus the initial 0).
            // Actually, in the loop: `prev` starts at 0.
            // For `[A, B, C]`:
            // 1. prev=0, byte=A. trans[0][A]++, sym[A]++, prev=A.
            // 2. prev=A, byte=B. trans[A][B]++, sym[B]++, prev=B.
            // 3. prev=B, byte=C. trans[B][C]++, sym[C]++, prev=C.
            
            // `symbol_counts` counts A, B, C.
            // We need count of how many times `prev` was the predecessor.
            // In the loop, `prev` takes values 0, A, B.
            // This is slightly different from `symbol_counts`.
            // However, for large files, it's approximately the same.
            // Let's re-calculate `prev_counts` strictly from transitions to be precise. 
            
            uint64_t total_transitions_from_x = 0;
            for(int y=0; y<256; ++y) total_transitions_from_x += result.transition_counts[x][y];
            
            if (total_transitions_from_x > 0) {
                double p_x = static_cast<double>(result.symbol_counts[x]) / total; // P(X=x)
                // Actually H(Y|X) = Sum P(X=x) * H(Y|X=x)
                // But here we are iterating over all transitions.
                // The formula used in Rust:
                // h_y_given_x += p_x * (Sum -p(y|x) log p(y|x))
                // where p_x is probability of the PREVIOUS symbol.
                // In my loop, `x` is the previous symbol index.
                // So I should use probability of `x` occurring as previous symbol?
                // The Rust code uses `symbol_freq.get(prev)` which is the global frequency of that byte.
                // I will stick to that to match the logic.
                
                double p_x_global = 0;
                if(result.symbol_counts[x] > 0) p_x_global = static_cast<double>(result.symbol_counts[x]) / total;
                
                // If the symbol exists in the file, calculate its conditional entropy contribution
                if (p_x_global > 0) {
                     double h_y_given_x_val = 0.0;
                     for (int y = 0; y < 256; ++y) {
                         uint64_t count_xy = result.transition_counts[x][y];
                         if (count_xy > 0) {
                             // Probability of transition x->y given x.
                             // Rust uses `symbol_freq` count for normalization of transitions.
                             double p_y_given_x = static_cast<double>(count_xy) / static_cast<double>(result.symbol_counts[x]);
                             h_y_given_x_val -= p_y_given_x * std::log2(p_y_given_x);
                         }
                     }
                     h_y_given_x += p_x_global * h_y_given_x_val;
                }
            }
        }

        std::cout << "=== ENTROPY ANALYSIS ===\n\n";
        std::cout << "File size: " << result.total_bytes << " bytes\n";
        
        int unique_symbols = 0;
        for(auto c : result.symbol_counts) if(c > 0) unique_symbols++;
        std::cout << "Unique symbols: " << unique_symbols << "\n\n";

        std::cout << "SYMBOL STATISTICS:\n";
        std::cout << std::left << std::setw(6) << "Byte" 
                  << std::setw(12) << "Count" 
                  << std::setw(12) << "Probability" << "\n";
        std::cout << std::string(30, '-') << "\n";

        std::vector<std::pair<uint8_t, uint64_t>> sorted_symbols;
        for(int i=0; i<256; ++i) {
            if(result.symbol_counts[i] > 0) sorted_symbols.push_back({(uint8_t)i, result.symbol_counts[i]});
        }
        std::sort(sorted_symbols.begin(), sorted_symbols.end(), [](const auto& a, const auto& b){
            return a.second > b.second;
        });

        int limit = std::min((int)sorted_symbols.size(), 32);
        for(int i=0; i<limit; ++i) {
            uint8_t byte = sorted_symbols[i].first;
            uint64_t count = sorted_symbols[i].second;
            double prob = static_cast<double>(count) / total;
            
            std::string char_repr;
            if (byte >= 32 && byte < 127) {
                char_repr = "'" + std::string(1, (char)byte) + "'";
            } else {
                char buffer[10];
                snprintf(buffer, sizeof(buffer), "0x%02x", byte);
                char_repr = buffer;
            }
            
            std::cout << std::left << std::setw(6) << char_repr 
                      << std::setw(12) << count 
                      << std::setw(12) << std::fixed << std::setprecision(6) << prob << "\n";
        }

        std::cout << "\n=== ENTROPY MEASURES ===\n";
        std::cout << "H(Y) - Total Entropy:              " << std::fixed << std::setprecision(6) << h_y << " bits\n";
        std::cout << "H(Y|X) - Conditional Entropy:      " << h_y_given_x << " bits\n";
        std::cout << "H(Y) - H(Y|X) - Information Gain:  " << (h_y - h_y_given_x) << " bits\n";

    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}
