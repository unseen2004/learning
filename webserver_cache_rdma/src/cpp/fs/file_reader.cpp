#include "../../headers/fs/file_reader.hpp"
#include <filesystem>
#include <fstream>

namespace fs = std::filesystem;

FileReadResult read_file(const std::string& path) {
  FileReadResult r;

  try {
    fs::path p(path);
    if (!fs::exists(p) || !fs::is_regular_file(p)) {
      r.ok = false; r.error = "File not found";
      return r;
    }
    auto fsize = fs::file_size(p);
    r.data.resize(static_cast<std::size_t>(fsize));

    std::ifstream ifs(path, std::ios::binary);
    if (!ifs) { r.ok = false; r.error = "Open failed"; return r; }
    if (!r.data.empty()) {
      ifs.read(reinterpret_cast<char*>(r.data.data()), static_cast<std::streamsize>(r.data.size()));
      if (!ifs) { r.ok = false; r.error = "Read failed"; return r; }
    }

    auto ftime = fs::last_write_time(p).time_since_epoch();
    auto secs = std::chrono::duration_cast<std::chrono::seconds>(ftime).count();
    r.last_modified = static_cast<std::time_t>(secs);

    r.ok = true;
    return r;
  } catch (const std::exception& ex) {
    r.ok = false; r.error = ex.what();
    return r;
  }
}