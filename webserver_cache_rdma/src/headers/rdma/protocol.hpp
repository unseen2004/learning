#pragma once
#include <cstdint>
#include <cstddef>
#include <string>
#include <vector>

namespace rdma_fast {

// Little-endian on the wire for simplicity

enum class Op : uint8_t {
  GET = 1,
  PING = 2,
};

#pragma pack(push, 1)
struct ReqHeader {
  uint8_t op;         // Op
  uint16_t path_len;  // bytes
};

struct RespHeader {
  uint16_t status;        // 200, 404, 500...
  uint64_t content_len;   // total payload bytes (0 on errors or PING)
  uint32_t chunk_size;    // size of subsequent SEND chunks (<= content_len)
};
#pragma pack(pop)

struct Request {
  Op op;
  std::string path; // for GET
};

inline bool parse_request(const char* data, std::size_t len, Request& out) {
  if (len < sizeof(ReqHeader)) return false;
  const auto* h = reinterpret_cast<const ReqHeader*>(data);
  uint16_t path_len = h->path_len;
  if (sizeof(ReqHeader) + path_len > len) return false;

  out.op = static_cast<Op>(h->op);
  if (out.op == Op::GET) {
    out.path.assign(data + sizeof(ReqHeader), data + sizeof(ReqHeader) + path_len);
  } else {
    out.path.clear();
  }
  return true;
}

inline std::vector<uint8_t> make_resp_header(uint16_t status, uint64_t content_len, uint32_t chunk) {
  std::vector<uint8_t> v(sizeof(RespHeader));
  auto* h = reinterpret_cast<RespHeader*>(v.data());
  h->status = status;
  h->content_len = content_len;
  h->chunk_size = chunk;
  return v;
}

} // namespace rdma_fast