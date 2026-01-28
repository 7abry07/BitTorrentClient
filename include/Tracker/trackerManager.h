#pragma once

#include <Tracker/httpTrackerSession.h>
#include <cstdint>
#include <string>
#include <unordered_map>
#include <vector>

namespace btc {

class Peer;

enum class eventType { None = 0, Completed, Started, Stopped };
enum class requestKind { Announce = 0, Scrape };

struct TrackerRequest {
  requestKind kind;
  std::string url;
  std::string trackerID;
  std::string infoHash;
  std::string pID;
  std::int64_t downloaded = -1;
  std::int64_t left = -1;
  std::int64_t uploaded = -1;
  std::int32_t ip = -1;
  std::uint32_t key = 0;
  std::uint16_t port = 0;
  bool compact = true;
  eventType event = eventType::None;
};

struct TrackerResponse {
  std::string failure;
  std::string warning;
  std::string trackerID;
  std::uint32_t interval = 0;
  std::uint32_t minInterval = 0;
  std::uint64_t complete = 0;
  std::uint64_t incomplete = 0;
  std::uint64_t downloaded = 0;
  std::vector<Peer> peerList;
};

class TrackerManager {

public:
  TrackerResponse send(TrackerRequest &req);

private:
  void createHttpSession();

  std::unordered_map<std::string, HttpTrackerSession> httpSessions;
};

} // namespace btc
