#include "../../headers/http/request.hpp"
#include "../../headers/http/headers.hpp"

std::string HttpRequest::header(const std::string& name) const {
  auto it = headers.find(header_lower(name));
  if (it != headers.end()) return it->second;
  return {};
}