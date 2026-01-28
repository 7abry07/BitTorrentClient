#include <Net/httpConnection.h>
#include <utils.h>

namespace btc {

std::expected<HttpConnection, BoostErrorCode>
HttpConnection::connect(IOContext &ctx, std::string hostname,
                        std::uint16_t port) {}

} // namespace btc
