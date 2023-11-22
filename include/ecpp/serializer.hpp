#ifndef ECPP_SERIAL_HPP_
#define ECPP_SERIAL_HPP_
#include <ecpp/endian.hpp>
#include <ecpp/serializer_options.hpp>

#include <algorithm>
#include <cstring>
#include <span>
#include <tuple>
#include <utility>
namespace ecpp {

    namespace serializer_impl {
        template<typename T> struct element_count_if_fixed : std::integral_constant<std::size_t, 0> {};
        template<typename T, std::size_t N> struct element_count_if_fixed<std::array<T, N>> : std::integral_constant<std::size_t, N> {};
        template<typename T, std::size_t N> struct element_count_if_fixed<T[N]> : std::integral_constant<std::size_t, N> {};
        template<typename T, std::size_t Extent>
            requires(Extent != std::dynamic_extent)
        struct element_count_if_fixed<std::span<T, Extent>> : std::integral_constant<std::size_t, Extent> {};

        template<class T> inline constexpr std::size_t element_count_if_fixed_v = element_count_if_fixed<T>::value;


        template<typename T> struct serializer;

        // Concept true for fundamental types
        template<typename T>
        concept fundamental = std::is_fundamental_v<std::remove_cvref_t<T>>;

        template<fundamental T> struct serializer<T> {
            constexpr static bool        has_fixed_size = true;
            constexpr static std::size_t fixed_size     = sizeof(T);

            template<bool CheckSize = false> constexpr static std::span<std::byte> serialize(T const& v, std::span<std::byte> in, opts o) noexcept {
                if constexpr (CheckSize) {
                    if (fixed_size > in.size())
                        return {};
                }

                std::memcpy(in.data(), &v, fixed_size);
                return in.subspan(fixed_size);
            }
        };

        template<typename T>
        concept range_iterable = std::is_array_v<T> || requires(T t) {
            {
                std::ranges::for_each(t, [](auto const&) {})
            };
        };


        template<range_iterable T> struct serializer<T> {
            using value_type        = std::remove_reference_t<decltype(*std::begin(std::declval<T&>()))>;
            using member_serializer = serializer<value_type>;

            constexpr static std::size_t fixed_size     = element_count_if_fixed_v<T> * member_serializer::fixed_size;
            constexpr static bool        has_fixed_size = (fixed_size != 0);

            template<bool CheckSize = not has_fixed_size> constexpr static std::span<std::byte> serialize(T const& v, std::span<std::byte> in, opts o) noexcept {
                if (CheckSize && member_serializer::has_fixed_size) {
                    auto req_space = std::size(v) * member_serializer::fixed_size;
                    if (req_space > in.size())
                        return {};
                }

                std::ranges::for_each(v, [&](auto const& e) { in = member_serializer::serialize(e, in, o); });
                return in;
            }
        };

        template<typename T>
        concept has_field_descr = requires { typename T::fields; };


        template<has_field_descr T> struct serializer<T> {
            consteval static bool members_have_fixed_size() noexcept {
                return std::apply([&](auto&&... args) { return ((serializer<member_t<args>>::fixed_size) && ...); }, T::fields::value);
            }

            consteval static std::size_t members_size() noexcept {
                return std::apply([&](auto&&... args) { return ((serializer<member_t<args>>::size) + ...); }, T::fields::value);
            }

            constexpr static bool        has_fixed_size = members_have_fixed_size();
            constexpr static std::size_t fixed_size     = has_fixed_size ? members_size() : 0;

            template<bool CheckSize = not has_fixed_size> constexpr static std::span<std::byte> serialize(T const& v, std::span<std::byte> buffer, opts o) noexcept {
                if constexpr (CheckSize) {
                    if (fixed_size > buffer.size())
                        return {};
                }
                std::apply([&](auto&&... args) { ((buffer = serializer<member_t<args>>::serialize(v.*args, buffer, o)), ...); }, T::fields::value);
                return buffer;
            }

          private:
            template<auto Ptr> using member_t = std::remove_cvref_t<decltype(std::declval<T>().*Ptr)>;
        };

    } // namespace serializer_impl

    template<auto... Ts> struct serializable_fields {
        constexpr static auto value = std::make_tuple(Ts...);
    };

    using typename serializer_impl::serializer;
    using serial_opts = serializer_impl::opts;

    template<typename T>
    // requires(requires { serializer<T>::serialize; })
    constexpr auto serialize(T const& v, std::span<std::byte> ar, serial_opts o = serial_opts::none) noexcept {
        auto remaining = serializer_impl::serializer<T>::template serialize<true>(v, ar, o);

        return ar.first(ar.size() - remaining.size());
    }

} // namespace ecpp

#endif
