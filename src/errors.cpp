#include <errors.h>

namespace btc {

const char *my_category_impl::name() const noexcept { return "btc"; }

std::string my_category_impl::message(int ev) const {
  char buffer[64];
  return message(ev, buffer, sizeof(buffer));
}

char const *my_category_impl::message(int ev, char *buffer,
                                      std::size_t len) const noexcept {
  error_code ec = static_cast<error_code>(ev);
  if (auto it = err_mess.find(ec); it != err_mess.end())
    return it->second.c_str();
  std::snprintf(buffer, len, "Unknown btc error %d", ev);
  return buffer;
}

sys::error_category const &my_category() {
  static const my_category_impl instance;
  return instance;
}

sys::error_code make_error_code(error_code e) {
  return sys::error_code(static_cast<int>(e), my_category());
}

} // namespace btc
