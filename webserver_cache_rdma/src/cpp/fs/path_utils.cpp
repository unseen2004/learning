#include "../../headers/fs/path_utils.hpp"
#include <filesystem>
#include <string>
#include <vector>

namespace fs = std::filesystem;

static std::string sanitize(const std::string& url_path) {
  auto p = url_path;
  auto qpos = p.find_first_of("?#");
  if (qpos != std::string::npos) p = p.substr(0, qpos);
  if (p.empty() || p[0] != '/') p = "/" + p;

  std::string out = "/";
  std::vector<std::string> parts;
  std::size_t i = 1;
  while (i <= p.size()) {
    auto j = p.find('/', i);
    if (j == std::string::npos) j = p.size();
    auto part = p.substr(i, j - i);
    if (!part.empty() && part != ".") {
      if (part == "..") { if (!parts.empty()) parts.pop_back(); }
      else { parts.push_back(part); }
    }
    i = j + 1;
  }
  for (size_t k = 0; k < parts.size(); ++k) {
    out += parts[k];
    if (k + 1 < parts.size()) out += "/";
  }
  return out;
}

PathMapResult map_url_to_fs(const std::string& doc_root, const std::string& url_path) {
  PathMapResult r;

  try {
    fs::path root = fs::weakly_canonical(fs::path(doc_root));
    if (!fs::exists(root) || !fs::is_directory(root)) {
      r.ok = false; r.exists = false; r.error = "Document root not found"; return r;
    }

    std::string sanitized = sanitize(url_path);
    fs::path rel = sanitized.substr(1);
    if (rel.empty()) rel = "index.html";

    fs::path target = root / rel;
    fs::path canon = fs::weakly_canonical(target);

    auto canon_root = root;
    if (canon.string().compare(0, canon_root.string().size(), canon_root.string()) != 0) {
      r.ok = false; r.exists = false; r.error = "Path traversal"; return r;
    }

    r.ok = true;
    r.exists = fs::exists(canon) && fs::is_regular_file(canon);
    r.fs_path = canon.string();
    r.cache_key = sanitized == "/" ? "/index.html" : sanitized;

    return r;
  } catch (const std::exception& ex) {
    r.ok = false; r.exists = false; r.error = ex.what();
    return r;
  }
}