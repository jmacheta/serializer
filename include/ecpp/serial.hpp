#ifndef ECPP_SERIAL_HPP_
#define ECPP_SERIAL_HPP_
#include <ecpp/endian.hpp>
#include <ecpp/serial_options.hpp>
#include <ecpp/serial_utils.hpp>

#include <array>
#include <cstdint>
#include <cstring>
#include <span>
#include <tuple>
#include <vector>
namespace ecpp {
    namespace serial_impl {

        enum class check_size {
            no,
            yes,
        };

        template<auto... Ts> struct serializable_fields {
            constexpr static auto value = std::make_tuple(Ts...);
        };

        using bytes = std::span<std::byte>;

        template<typename T> struct serializer;

        template<fundamental T> struct serializer<T> {
            constexpr static bool fixed_size = true;

            constexpr static std::size_t size = sizeof(T);

            template<bool CheckSize = false> constexpr static bytes serialize(T const& v, bytes in, opts o) noexcept {
                if constexpr (CheckSize) {
                    if (size > in.size())
                        return bytes{};
                }

                std::memcpy(in.data(), &v, size);
                return in.subspan(size);
            }
        };

        template<array T> struct serializer<T> {
            using value_type              = std::remove_reference_t<decltype(*std::declval<T>())>;
            constexpr static size_t count = sizeof(T) / sizeof(value_type);

            constexpr static bool        fixed_size = serializer<value_type>::fixed_size;
            constexpr static std::size_t size       = count * serializer<value_type>::size;

            template<bool CheckSize = false> constexpr static bytes serialize(T const& v, bytes in, opts o) noexcept {
                // There is no point in checking dynamic value size beforehand, as it
                // guarantees nothing
                if constexpr (CheckSize && fixed_size) {
                    if (size > in.size())
                        return bytes{};
                }

                // If the values are dynamic, the check will be performed either way. If
                // values are fixed, the default is to ommit the check
                std::ranges::for_each(v, [&](auto const& e) { in = serializer<value_type>::serialize(e, in, o); });
                return in;
            }
        };

        template<class T, class Allocator> struct serializer<std::vector<T, Allocator>> {
            using container_type                    = std::vector<T, Allocator>;
            using value_type                        = typename container_type::value_type;
            constexpr static bool        fixed_size = serializer<value_type>::fixed_size;
            constexpr static std::size_t size       = 0;

            template<bool CheckSize = true> constexpr static bytes serialize(container_type const& v, bytes in, opts o) noexcept {
                if (CheckSize && fixed_size) {
                    auto req_space = v.size() * serializer<value_type>::size;
                    if (req_space > in.size())
                        return bytes{};
                }

                std::ranges::for_each(v, [&](auto const& e) { in = serializer<value_type>::serialize(e, in, o); });
                return in;
            }
        };

        template<class T, std::size_t N> struct serializer<std::array<T, N>> {
            using container_type                    = std::array<T, N>;
            using value_type                        = typename container_type::value_type;
            constexpr static bool        fixed_size = serializer<value_type>::fixed_size;
            constexpr static std::size_t size       = N * serializer<value_type>::size;

            template<bool CheckSize = false> constexpr static bytes serialize(container_type const& v, bytes in, opts o) noexcept {
                if (CheckSize && fixed_size) {
                    auto req_space = v.size() * serializer<value_type>::size;
                    if (req_space > in.size())
                        return bytes{};
                }

                std::ranges::for_each(v, [&](auto const& e) { in = serializer<value_type>::serialize(e, in, o); });
                return in;
            }
        };

        template<has_field_descr T> struct serializer<T> {
            consteval static bool members_have_fixed_size() noexcept {
                return std::apply([&](auto&&... args) { return ((serializer<member_t<args>>::fixed_size) && ...); }, T::fields::value);
            }

            consteval static std::size_t members_size() noexcept {
                return std::apply([&](auto&&... args) { return ((serializer<member_t<args>>::size) + ...); }, T::fields::value);
            }

            constexpr static bool        fixed_size = members_have_fixed_size();
            constexpr static std::size_t size       = members_size();

            template<bool CheckSize = not fixed_size> constexpr static bytes serialize(T const& v, bytes buffer, opts o) noexcept {
                if constexpr (CheckSize) {
                    if (size > buffer.size())
                        return bytes{};
                }
                std::apply([&](auto&&... args) { ((buffer = serializer<member_t<args>>::serialize(v.*args, buffer, o)), ...); }, T::fields::value);
                return buffer;
            }

          private:
            template<auto Ptr> using member_t = std::remove_cvref_t<decltype(std::declval<T>().*Ptr)>;
        };

    } // namespace serial_impl

    using typename serial_impl::serializable_fields;
    using typename serial_impl::serializer;
    using serial_opts = serial_impl::opts;

    template<typename T>
    // requires(requires { serializer<T>::serialize; })
    constexpr auto serialize(T const& v, std::span<std::byte> ar, serial_opts o = serial_opts::none) noexcept {
        auto remaining = serial_impl::serializer<T>::serialize(v, ar, o);

        return ar.first(ar.size() - remaining.size());
    }

} // namespace ecpp

#endif
