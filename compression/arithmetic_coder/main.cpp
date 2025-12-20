#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <stdexcept>
#include <cstdint>
#include <cmath>
#include <iomanip>
#include <algorithm>

namespace {

constexpr int PRECISION_BITS = 32;
using int_type = uint64_t;
constexpr int_type TOP_VALUE = (static_cast<int_type>(1) << PRECISION_BITS) - 1;
constexpr int_type FIRST_QUARTER = (TOP_VALUE / 4) + 1;
constexpr int_type HALF = 2 * FIRST_QUARTER;
constexpr int_type THIRD_QUARTER = 3 * FIRST_QUARTER;

constexpr uint16_t EOF_SYMBOL = 256;
constexpr uint16_t NUM_SYMBOLS = 257;

constexpr uint32_t MAX_TOTAL_COUNT = 1 << 16;

class BitWriter {
    std::ostream& os;
    uint8_t buffer = 0;
    int bit_count = 0;
    uint64_t total_bits_written = 0;
    uint64_t total_bytes_written = 0;

    void flush_buffer() {
        if (bit_count > 0) {
            os.put(buffer);
            total_bytes_written++;
            buffer = 0;
            bit_count = 0;
        }
    }

public:
    explicit BitWriter(std::ostream& out) : os(out) {}

    ~BitWriter() {
        flush();
    }

    void write(int bit) {
        buffer = (buffer << 1) | (bit & 1);
        bit_count++;
        total_bits_written++;
        if (bit_count == 8) {
            flush_buffer();
        }
    }

    void flush() {
        if (bit_count > 0) {
            buffer <<= (8 - bit_count);
            flush_buffer();
        }
    }

    uint64_t get_total_bits() const { return total_bits_written; }
    uint64_t get_total_bytes() const { return total_bytes_written; }
};

class BitReader {
    std::istream& is;
    uint8_t buffer = 0;
    int bit_count = 0;

    int read_bit() {
        if (bit_count == 0) {
            int ch = is.get();
            if (ch == EOF) {
                return 0;
            }
            buffer = static_cast<uint8_t>(ch);
            bit_count = 8;
        }
        bit_count--;
        return (buffer >> bit_count) & 1;
    }

public:
    explicit BitReader(std::istream& in) : is(in) {}

    int read() {
        return read_bit();
    }
};

class AdaptiveModel {
    std::vector<uint32_t> tree;
    uint32_t total_count = 0;

    void update_tree(int index, int delta) {
        index++;
        while (index < tree.size()) {
            tree[index] += delta;
            index += index & (-index);
        }
    }

    uint32_t query_tree(int index) const {
        index++;
        uint32_t sum = 0;
        while (index > 0) {
            sum += tree[index];
            index -= index & (-index);
        }
        return sum;
    }

    void rescale() {
        total_count = 0;
        std::vector<uint32_t> old_tree = tree;
        std::fill(tree.begin(), tree.end(), 0);

        for (int symbol = 0; symbol < NUM_SYMBOLS; ++symbol) {
            uint32_t old_freq = query_tree(symbol, old_tree) - query_tree(symbol - 1, old_tree);
            uint32_t new_freq = (old_freq / 2) + 1;

            update_tree(symbol, new_freq);
            total_count += new_freq;
        }
    }

    uint32_t query_tree(int index, const std::vector<uint32_t>& t) const {
        if (index < 0) return 0;
        index++;
        uint32_t sum = 0;
        while (index > 0) {
            sum += t[index];
            index -= index & (-index);
        }
        return sum;
    }


public:
    AdaptiveModel() : tree(NUM_SYMBOLS + 1, 0) {
        for (int i = 0; i < NUM_SYMBOLS; ++i) {
            update(i);
        }
    }

    void update(int symbol) {
        update_tree(symbol, 1);
        total_count++;
        if (total_count >= MAX_TOTAL_COUNT) {
            rescale();
        }
    }

    uint32_t get_total() const {
        return total_count;
    }

    void get_range(int symbol, uint32_t& low, uint32_t& high) const {
        low = query_tree(symbol - 1);
        high = query_tree(symbol);
    }

    int get_symbol(uint32_t target_count, uint32_t& low, uint32_t& high) const {
        int symbol = 0;
        int min_s = 0;
        int max_s = NUM_SYMBOLS - 1;

        while (min_s <= max_s) {
            int mid = min_s + (max_s - min_s) / 2;
            uint32_t mid_high = query_tree(mid);
            if (mid_high <= target_count) {
                symbol = mid;
                min_s = mid + 1;
            } else {
                max_s = mid - 1;
            }
        }

        while (query_tree(symbol) <= target_count) {
            symbol++;
        }

        get_range(symbol, low, high);
        return symbol;
    }
};

class ArithmeticEncoder {
    BitWriter& writer;
    AdaptiveModel model;
    int_type low;
    int_type high;
    int pending_bits;

    void output_bit_plus_pending(int bit) {
        writer.write(bit);
        for (int i = 0; i < pending_bits; ++i) {
            writer.write(!bit);
        }
        pending_bits = 0;
    }

    void scale() {
        while (true) {
            if (high < HALF) {
                output_bit_plus_pending(0);
                low = 2 * low;
                high = 2 * high + 1;
            }
            else if (low >= HALF) {
                output_bit_plus_pending(1);
                low = 2 * (low - HALF);
                high = 2 * (high - HALF) + 1;
            }
            else if (low >= FIRST_QUARTER && high < THIRD_QUARTER) {
                pending_bits++;
                low = 2 * (low - FIRST_QUARTER);
                high = 2 * (high - FIRST_QUARTER) + 1;
            }
            else {
                break;
            }
        }
    }

public:
    long double total_information = 0.0;
    uint64_t total_symbols = 0;

    explicit ArithmeticEncoder(BitWriter& bw)
        : writer(bw), low(0), high(TOP_VALUE), pending_bits(0) {}

    void encode(int symbol) {
        uint32_t sym_low, sym_high, total;
        total = model.get_total();
        model.get_range(symbol, sym_low, sym_high);

        long double p = static_cast<long double>(sym_high - sym_low) / total;
        total_information += -log2l(p);
        total_symbols++;

        const int_type range = high - low + 1;
        high = low + (range * sym_high) / total - 1;
        low = low + (range * sym_low) / total;

        scale();
        model.update(symbol);
    }

    void flush() {
        pending_bits++;
        if (low < FIRST_QUARTER) {
            output_bit_plus_pending(0);
        } else {
            output_bit_plus_pending(1);
        }
        writer.flush();
    }
};

class ArithmeticDecoder {
    BitReader& reader;
    AdaptiveModel model;
    int_type low;
    int_type high;
    int_type value;

    void scale() {
        while (true) {
            if (high < HALF) {
                low = 2 * low;
                high = 2 * high + 1;
                value = 2 * value + reader.read();
            } else if (low >= HALF) {
                low = 2 * (low - HALF);
                high = 2 * (high - HALF) + 1;
                value = 2 * (value - HALF) + reader.read();
            } else if (low >= FIRST_QUARTER && high < THIRD_QUARTER) {
                low = 2 * (low - FIRST_QUARTER);
                high = 2 * (high - FIRST_QUARTER) + 1;
                value = 2 * (value - FIRST_QUARTER) + reader.read();
            } else {
                break;
            }
        }
    }

public:
    explicit ArithmeticDecoder(BitReader& br)
        : reader(br), low(0), high(TOP_VALUE), value(0) {
        for (int i = 0; i < PRECISION_BITS; ++i) {
            value = (value << 1) | reader.read();
        }
    }

    int decode() {
        const int_type range = high - low + 1;
        const uint32_t total = model.get_total();
        const uint32_t target_count = static_cast<uint32_t>(((value - low + 1) * total - 1) / range);

        uint32_t sym_low, sym_high;
        int symbol = model.get_symbol(target_count, sym_low, sym_high);

        if (symbol == EOF_SYMBOL) {
            return EOF_SYMBOL;
        }

        high = low + (range * sym_high) / total - 1;
        low = low + (range * sym_low) / total;

        scale();
        model.update(symbol);

        return symbol;
    }
};

uint64_t get_file_size(const std::string& filename) {
    std::ifstream file(filename, std::ios::binary | std::ios::ate);
    if (!file) return 0;
    return static_cast<uint64_t>(file.tellg());
}

void encode_file(const std::string& input_filename, const std::string& output_filename) {
    std::ifstream ifs(input_filename, std::ios::binary);
    if (!ifs) throw std::runtime_error("Cannot open input file: " + input_filename);

    std::ofstream ofs(output_filename, std::ios::binary);
    if (!ofs) throw std::runtime_error("Cannot open output file: " + output_filename);

    uint64_t input_file_size = get_file_size(input_filename);
    if (input_file_size == 0 && !ifs.peek()) {
        std::cout << "Input file is empty. Creating empty output file." << std::endl;
        return;
    }

    BitWriter writer(ofs);
    ArithmeticEncoder encoder(writer);

    std::cout << "Encoding file: " << input_filename << std::endl;

    int byte;
    while ((byte = ifs.get()) != EOF) {
        encoder.encode(static_cast<uint8_t>(byte));
    }
    encoder.encode(EOF_SYMBOL);
    encoder.flush();

    uint64_t output_file_size = writer.get_total_bytes();
    uint64_t total_bits = writer.get_total_bits();
    uint64_t total_symbols = encoder.total_symbols;
    long double entropy = encoder.total_information / total_symbols;
    long double avg_length = static_cast<long double>(total_bits) / total_symbols;
    long double ratio = static_cast<long double>(output_file_size) / input_file_size;

    std::cout << std::fixed << std::setprecision(6);
    std::cout << "--- Encoding Statistics ---" << std::endl;
    std::cout << "Input size:    " << input_file_size << " bytes" << std::endl;
    std::cout << "Output size:   " << output_file_size << " bytes" << std::endl;
    std::cout << "Model entropy: " << entropy << " bits/symbol" << std::endl;
    std::cout << "Avg length:    " << avg_length << " bits/symbol" << std::endl;
    std::cout << "Ratio:         " << ratio << " (" << ratio * 100.0 << "%)" << std::endl;
}

void decode_file(const std::string& input_filename, const std::string& output_filename) {
    std::ifstream ifs(input_filename, std::ios::binary);
    if (!ifs) throw std::runtime_error("Cannot open input file: " + input_filename);

    if (get_file_size(input_filename) == 0) {
        std::cout << "Input file is empty. Creating empty output file." << std::endl;
        std::ofstream ofs(output_filename, std::ios::binary);
        return;
    }

    std::ofstream ofs(output_filename, std::ios::binary);
    if (!ofs) throw std::runtime_error("Cannot open output file: " + output_filename);

    BitReader reader(ifs);
    ArithmeticDecoder decoder(reader);

    std::cout << "Decoding file: " << input_filename << std::endl;

    while (true) {
        int symbol = decoder.decode();
        if (symbol == EOF_SYMBOL) {
            break;
        }
        ofs.put(static_cast<char>(symbol));
    }

    std::cout << "Decoding complete." << std::endl;
}

}

int main(int argc, char* argv[]) {
    if (argc != 4) {
        std::cerr << "Usage: " << argv[0] << " <-c | -d> <input_file> <output_file>" << std::endl;
        std::cerr << "  -c : compress (encode)" << std::endl;
        std::cerr << "  -d : decompress (decode)" << std::endl;
        return 1;
    }

    std::string mode = argv[1];
    std::string input_file = argv[2];
    std::string output_file = argv[3];

    try {
        if (mode == "-c") {
            encode_file(input_file, output_file);
        } else if (mode == "-d") {
            decode_file(input_file, output_file);
        } else {
            std::cerr << "Unknown mode: " << mode << std::endl;
            return 1;
        }
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}
