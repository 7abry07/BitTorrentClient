#pragma once

#include <boost/system.hpp>
#include <boost/system/detail/error_category.hpp>
#include <boost/system/detail/error_code.hpp>
#include <error_codes.h>
#include <helpers.h>
#include <string>

namespace btc {
sys::error_code make_error_code(error_code e);

class my_category_impl : public sys::error_category {

public:
  const char *name() const noexcept;
  std::string message(int ev) const;
  char const *message(int ev, char *buffer, std::size_t len) const noexcept;
  sys::error_category const &my_category();
};

} // namespace btc

namespace std {
template <> struct is_error_code_enum<::btc::error_code> : std::true_type {};
} // namespace std

namespace boost {
namespace system {
template <> struct is_error_code_enum<::btc::error_code> : std::true_type {};
} // namespace system
} // namespace boost
