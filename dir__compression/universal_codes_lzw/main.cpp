#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <unordered_map>
#include <cmath>
#include <map>
#include <algorithm>
#include <functional>

class BitStream {
public:
    explicit BitStream(std::ostream& os) : os_(&os), buffer_(0), bit_count_(0) {}
    explicit BitStream(std::istream& is) : is_(&is), buffer_(0), bit_count_(0) {}

    void write_bit(int bit) {
        buffer_ = (buffer_ << 1) | (bit & 1);
        bit_count_++;
        if (bit_count_ == 8) {
            os_->put(static_cast<char>(buffer_));
            bit_count_ = 0;
            buffer_ = 0;
        }
    }

    int read_bit() {
        if (bit_count_ == 0) {
            int byte = is_->get();
            if (byte == EOF) {
                return EOF;
            }
            buffer_ = static_cast<unsigned char>(byte);
            bit_count_ = 8;
        }
        bit_count_--;
        return (buffer_ >> bit_count_) & 1;
    }

    void flush() {
        while (bit_count_ > 0) {
            write_bit(0);
        }
    }

private:
    std::ostream* os_ = nullptr;
    std::istream* is_ = nullptr;
    unsigned char buffer_;
    int bit_count_;
};

void encode_gamma(BitStream& bs, unsigned long long n) {
    if (n == 0) return;
    std::string binary_n;
    unsigned long long temp_n = n;
    while (temp_n > 0) {
        binary_n += (temp_n % 2 == 0 ? "0" : "1");
        temp_n /= 2;
    }
    std::reverse(binary_n.begin(), binary_n.end());
    
    int len = binary_n.length();
    for (int i = 0; i < len - 1; ++i) {
        bs.write_bit(0);
    }
    for (char bit : binary_n) {
        bs.write_bit(bit - '0');
    }
}

unsigned long long decode_gamma(BitStream& bs) {
    int num_zeros = 0;
    int bit;
    while ((bit = bs.read_bit()) == 0) {
        num_zeros++;
    }
    if (bit == EOF) return 0;

    unsigned long long value = 1;
    for (int i = 0; i < num_zeros; ++i) {
        bit = bs.read_bit();
        if (bit == EOF) return 0;
        value = (value << 1) | bit;
    }
    return value;
}

void encode_delta(BitStream& bs, unsigned long long n) {
    if (n == 0) return;
    std::string binary_n;
    unsigned long long temp_n = n;
    while (temp_n > 0) {
        binary_n += (temp_n % 2 == 0 ? "0" : "1");
        temp_n /= 2;
    }
    std::reverse(binary_n.begin(), binary_n.end());
    
    int len = binary_n.length();
    encode_gamma(bs, len);
    for (size_t i = 1; i < binary_n.length(); ++i) {
        bs.write_bit(binary_n[i] - '0');
    }
}

unsigned long long decode_delta(BitStream& bs) {
    unsigned long long len = decode_gamma(bs);
    if (len == 0) return 0;

    unsigned long long value = 1;
    for (unsigned long long i = 0; i < len - 1; ++i) {
        int bit = bs.read_bit();
        if (bit == EOF) return 0;
        value = (value << 1) | bit;
    }
    return value;
}

void encode_omega(BitStream& bs, unsigned long long n) {
    if (n == 0) return;
    std::string code = "0";
    unsigned long long k = n;
    while (k > 1) {
        std::string binary_k;
        unsigned long long temp_k = k;
        while (temp_k > 0) {
            binary_k += (temp_k % 2 == 0 ? "0" : "1");
            temp_k /= 2;
        }
        std::reverse(binary_k.begin(), binary_k.end());
        code = binary_k + code;
        k = binary_k.length() - 1;
    }
    for (char bit : code) {
        bs.write_bit(bit - '0');
    }
}

unsigned long long decode_omega(BitStream& bs) {
    unsigned long long n = 1;
    while (true) {
        int bit = bs.read_bit();
        if (bit == 0) {
            return n;
        }
        if (bit == EOF) {
            return 0;
        }

        unsigned long long len_to_read = n;
        unsigned long long next_n = 1;
        for (unsigned long long i = 0; i < len_to_read; ++i) {
            bit = bs.read_bit();
            if (bit == EOF) return 0;
            next_n = (next_n << 1) | bit;
        }
        n = next_n;
    }
}

void encode_fibonacci(BitStream& bs, unsigned long long n) {
    if (n == 0) return;
    std::vector<unsigned long long> fib;
    fib.push_back(1);
    fib.push_back(2);
    while (fib.back() <= n) {
        fib.push_back(fib[fib.size() - 1] + fib[fib.size() - 2]);
    }
    if (!fib.empty()) {
        fib.pop_back();
    }

    std::string code;
    unsigned long long remainder = n;
    for (int i = fib.size() - 1; i >= 0; --i) {
        if (remainder >= fib[i]) {
            code += '1';
            remainder -= fib[i];
        } else {
            code += '0';
        }
    }
    std::reverse(code.begin(), code.end());
    code += '1';
    for (char bit : code) {
        bs.write_bit(bit - '0');
    }
}

unsigned long long decode_fibonacci(BitStream& bs) {
    std::vector<int> bits;
    int bit;
    bool prev_bit_one = false;
    while (true) {
        bit = bs.read_bit();
        if (bit == EOF) return 0;
        bits.push_back(bit);
        if (prev_bit_one && bit == 1) {
            break;
        }
        prev_bit_one = (bit == 1);
    }
    if (bits.empty()) return 0;
    bits.pop_back();

    unsigned long long n = 0;
    unsigned long long f1 = 1, f2 = 1;
    for (size_t i = 0; i < bits.size(); ++i) {
        if (bits[i] == 1) {
            n += f1;
        }
        unsigned long long temp = f1 + f2;
        f1 = f2;
        f2 = temp;
    }
    return n;
}

void lzw_encode(std::istream& input, std::ostream& output, const std::function<void(BitStream&, unsigned long long)>& encoder) {
    std::unordered_map<std::string, unsigned long long> dictionary;
    for (int i = 0; i < 256; ++i) {
        dictionary[std::string(1, static_cast<char>(i))] = i + 1;
    }
    unsigned long long next_code = 257;

    BitStream bs(output);
    std::string current_string;
    char c;
    while (input.get(c)) {
        std::string next_string = current_string + c;
        if (dictionary.count(next_string)) {
            current_string = next_string;
        } else {
            encoder(bs, dictionary[current_string]);
            if (next_code < 500000) { 
               dictionary[next_string] = next_code++;
            }
            current_string = std::string(1, c);
        }
    }
    if (!current_string.empty()) {
        encoder(bs, dictionary[current_string]);
    }
    bs.flush();
}

void lzw_decode(std::istream& input, std::ostream& output, const std::function<unsigned long long(BitStream&)>& decoder) {
    std::unordered_map<unsigned long long, std::string> dictionary;
    for (int i = 0; i < 256; ++i) {
        dictionary[i + 1] = std::string(1, static_cast<char>(i));
    }
    unsigned long long next_code = 257;

    BitStream bs(input);
    std::string previous_string;
    unsigned long long code;

    while ((code = decoder(bs)) != 0) {
        std::string current_string;
        if (dictionary.count(code)) {
            current_string = dictionary[code];
        } else if (code == next_code) {
            current_string = previous_string + previous_string[0];
        } else {
            std::cerr << "Error: Invalid code " << code << std::endl;
            break; 
        }

        output << current_string;

        if (!previous_string.empty()) {
             if (next_code < 500000) {
                dictionary[next_code++] = previous_string + current_string[0];
             }
        }
        previous_string = current_string;
    }
}

double calculate_entropy(std::istream& is) {
    std::map<char, long long> counts;
    long long total = 0;
    
    is.clear();
    is.seekg(0, std::ios::beg);
    
    char c;
    while (is.get(c)) {
        counts[c]++;
        total++;
    }

    if (total == 0) return 0.0;

    double entropy = 0.0;
    for (auto const& [key, val] : counts) {
        double p = static_cast<double>(val) / total;
        entropy -= p * std::log2(p);
    }
    
    is.clear();
    is.seekg(0, std::ios::beg);
    return entropy;
}

long long get_file_size(const std::string& filename) {
    std::ifstream file(filename, std::ios::binary | std::ios::ate);
    if (!file.is_open()) return 0;
    return file.tellg();
}

int main(int argc, char* argv[]) {
    if (argc < 4) {
        std::cerr << "Usage: " << argv[0] << " <encode|decode> <input_file> <output_file> [omega|gamma|delta|fibonacci]" << std::endl;
        return 1;
    }

    std::string mode = argv[1];
    std::string input_filename = argv[2];
    std::string output_filename = argv[3];
    std::string coding_type = "omega";
    if (argc > 4) {
        coding_type = argv[4];
    }

    std::function<void(BitStream&, unsigned long long)> encoder;
    std::function<unsigned long long(BitStream&)> decoder;

    if (coding_type == "omega") {
        encoder = encode_omega;
        decoder = decode_omega;
    } else if (coding_type == "gamma") {
        encoder = encode_gamma;
        decoder = decode_gamma;
    } else if (coding_type == "delta") {
        encoder = encode_delta;
        decoder = decode_delta;
    } else if (coding_type == "fibonacci") {
        encoder = encode_fibonacci;
        decoder = decode_fibonacci;
    } else {
        std::cerr << "Unknown coding type: " << coding_type << std::endl;
        return 1;
    }

    std::ifstream input_file(input_filename, std::ios::binary);
    if (!input_file) {
        std::cerr << "Error opening input file: " << input_filename << std::endl;
        return 1;
    }

    std::ofstream output_file(output_filename, std::ios::binary);
    if (!output_file) {
        std::cerr << "Error opening output file: " << output_filename << std::endl;
        return 1;
    }

    if (mode == "encode") {
        double input_entropy = calculate_entropy(input_file);
        
        lzw_encode(input_file, output_file, encoder);
        
        input_file.close();
        output_file.close();

        long long input_size = get_file_size(input_filename);
        long long output_size = get_file_size(output_filename);
        
        std::ifstream encoded_file(output_filename, std::ios::binary);
        double output_entropy = calculate_entropy(encoded_file);

        std::cout << "Input file size: " << input_size << " bytes" << std::endl;
        std::cout << "Output file size: " << output_size << " bytes" << std::endl;
        if (input_size > 0) {
            std::cout << "Compression ratio: " << static_cast<double>(output_size) / input_size << std::endl;
        }
        std::cout << "Input file entropy: " << input_entropy << std::endl;
        std::cout << "Output code entropy: " << output_entropy << std::endl;

    } else if (mode == "decode") {
        lzw_decode(input_file, output_file, decoder);
        std::cout << "Decoding complete." << std::endl;
    } else {
        std::cerr << "Unknown mode: " << mode << std::endl;
        return 1;
    }

    return 0;
}
