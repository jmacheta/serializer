#ifndef ECPP_SERIALIZER_OPTIONS_HPP_
#define ECPP_SERIALIZER_OPTIONS_HPP_

#include <type_traits>

namespace ecpp {
    namespace serializer_impl {

        enum class opts {
            none          = 0,
            big_endian    = 1,
            little_endian = 2,
        };

        using underlying = typename std::underlying_type<opts>::type;

        constexpr bool has_flag(opts value, opts flag) noexcept {
            auto u_flag = static_cast<underlying>(flag);
            return u_flag == (static_cast<underlying>(value) & u_flag);
        }

        constexpr bool big_endian_opt(opts o) noexcept {
            return has_flag(o, opts::big_endian);
        }

        constexpr opts operator|(opts lhs, opts rhs) noexcept {
            return static_cast<opts>(static_cast<underlying>(lhs) | static_cast<underlying>(rhs));
        }

    } // namespace serializer_impl
} // namespace ecpp
#endif