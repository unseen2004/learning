#include "../../headers/http/mime.hpp"
#include <string>
#include <unordered_map>

static std::string ext_of(const std::string& path) {
  auto pos = path.find_last_of('.');
  if (pos == std::string::npos) return {};
  std::string ext = path.substr(pos + 1);
  for (auto& c : ext) c = static_cast<char>(std::tolower(static_cast<unsigned char>(c)));
  return ext;
}

std::string mime_type(const std::string& path) {
  static const std::unordered_map<std::string, std::string> m = {
    {"html","text/html; charset=utf-8"},
    {"htm","text/html; charset=utf-8"},
    {"css","text/css"},
    {"js","application/javascript"},
    {"json","application/json"},
    {"png","image/png"},
    {"jpg","image/jpeg"},
    {"jpeg","image/jpeg"},
    {"gif","image/gif"},
    {"svg","image/svg+xml"},
    {"txt","text/plain; charset=utf-8"},
    {"xml","application/xml"},
    {"pdf","application/pdf"},
    {"wasm","application/wasm"}
  };
  auto ext = ext_of(path);
  auto it = m.find(ext);
  if (it != m.end()) return it->second;
  return "application/octet-stream";
}