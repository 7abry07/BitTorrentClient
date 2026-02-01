#pragma once

#include <helpers.h>
#include <optional>
#include <string>

namespace btc {

struct Peer {
  std::optional<std::string> pID = "";
  std::string ip = "";
  port_t port = 0;
};

} // namespace btc
