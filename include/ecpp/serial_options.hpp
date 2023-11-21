#ifndef ECPP_SERIAL_OPTIONS_HPP_
#define ECPP_SERIAL_OPTIONS_HPP_

#include <type_traits>

namespace ecpp {
namespace serial_impl {

enum class opts {
    none = 0,
    big_endian = 1,
    little_endian = 2,
    no_size_check = 4,
};

using underlying = typename std::underlying_type<opts>::type;

constexpr bool has_flag(opts value, opts flag) noexcept {
    auto u_flag = static_cast<underlying>(flag);
    return u_flag == (static_cast<underlying>(value) & u_flag);
}

constexpr bool big_endian_opt(opts o) noexcept {
    return has_flag(o, opts::big_endian);
}

constexpr bool no_size_check_opt(opts o) noexcept {
    return has_flag(o, opts::no_size_check);
}

constexpr opts operator|(opts lhs, opts rhs) noexcept {
    return static_cast<opts>(static_cast<underlying>(lhs) |
                             static_cast<underlying>(rhs));
}

}  // namespace serial_impl
}  // namespace ecpp
#endif