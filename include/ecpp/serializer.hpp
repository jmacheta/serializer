#ifndef ECPP_SERIAL_HPP_
#define ECPP_SERIAL_HPP_
#include <ecpp/endian.hpp>

#include <algorithm>
#include <cstring>
#include <span>
#include <tuple>
#include <utility>

namespace ecpp {

    namespace serializer_impl {
        /** @brief Provides access to number of elements in a fixed size containers, i.e. c-array, std::array, and std::span with non-dynamic extent. For other types the value is always 0
         * @tparam T Type of the container.
         */
        template<typename T> struct element_count_if_fixed : std::integral_constant<std::size_t, 0> {};

        template<typename T, std::size_t N> struct element_count_if_fixed<std::array<T, N>> : std::integral_constant<std::size_t, N> {};
        template<typename T, std::size_t N> struct element_count_if_fixed<T[N]> : std::integral_constant<std::size_t, N> {};
        template<typename T, std::size_t Extent>
            requires(Extent != std::dynamic_extent)
        struct element_count_if_fixed<std::span<T, Extent>> : std::integral_constant<std::size_t, Extent> {};

        /// Concept true for fundamental types
        template<typename T>
        concept fundamental = std::is_fundamental_v<std::remove_cvref_t<T>>;

        /// Concept true for containers that can be iterated over with std::ranges::for_each
        template<typename T>
        concept range_iterable = std::is_array_v<T> || requires(T t) {
            {
                std::ranges::for_each(t, [](auto const&) {})
            };
        };

        /// Concept true for types that have a static member named fields, which is a tuple of pointers to members of the type.
        template<typename T>
        concept has_field_descr = requires { typename T::fields; };

    } // namespace serializer_impl

    /// @brief Options for the serializer
    enum class serializer_options {
        none          = 0,
        big_endian    = 1,
        little_endian = 2,
    };

    /// @brief Concatenates two serializer_options values
    /// @param lhs lhs
    /// @param rhs rhs
    /// @return lhs | rhs
    constexpr serializer_options operator|(serializer_options lhs, serializer_options rhs) noexcept {
        using u_t = typename std::underlying_type<serializer_options>::type;
        return static_cast<serializer_options>(static_cast<u_t>(lhs) | static_cast<u_t>(rhs));
    }


    template<typename T> struct serializer;

    /// @brief Provides serialization for fundamental types
    /// @tparam fundamental type (see @ref serializer_impl::fundamental)
    template<serializer_impl::fundamental T> struct serializer<T> {
        constexpr static bool        has_fixed_size = true;      ///< Informs that the size of the type is fixed
        constexpr static std::size_t fixed_size     = sizeof(T); ///< Size of the serialized object in bytes

        template<bool CheckSize = false> constexpr static std::span<std::byte> serialize(T const& v, std::span<std::byte> in, serializer_options o) noexcept {
            if constexpr (CheckSize) {
                if (fixed_size > in.size())
                    return {};
            }

            if (o == serializer_options::big_endian) {
                // todo
            }


            std::memcpy(in.data(), &v, fixed_size);
            return in.subspan(fixed_size);
        }
    };

    /// @brief Provides serialization for types that can be iterated over with std::ranges::for_each, e.g. std::array, std::span, std::vector, c-array
    /// @tparam T type iterable with std::ranges::for_each (see @ref serializer_impl::range_iterable)
    template<serializer_impl::range_iterable T> struct serializer<T> {
        using value_type        = std::remove_reference_t<decltype(*std::begin(std::declval<T&>()))>;
        using member_serializer = serializer<value_type>;

        /// The size of the object depends on, whether the container has fixed size, and the size of the value_type is fixed. If either of these is not true, the size is not fixed (set to 0).
        constexpr static std::size_t fixed_size = serializer_impl::element_count_if_fixed<T>::value * member_serializer::fixed_size;

        /// Informs whether the size of the object is fixed
        constexpr static bool has_fixed_size = (fixed_size != 0);

        template<bool CheckSize = not has_fixed_size> constexpr static std::span<std::byte> serialize(T const& v, std::span<std::byte> in, serializer_options o) noexcept {
            // The check below makes sense only, when the value_type size is fixed.
            if (CheckSize && member_serializer::has_fixed_size) {
                auto req_space = std::size(v) * member_serializer::fixed_size;
                if (req_space > in.size())
                    return {};
            }

            std::ranges::for_each(v, [&](auto const& e) { in = member_serializer::serialize(e, in, o); });
            return in;
        }
    };

    /// @brief Provides generic serialization for composite types
    /// @tparam T type with a static member named fields, which is a tuple of pointers to members of the type (see @ref serializer_impl::has_field_descr)
    template<serializer_impl::has_field_descr T> struct serializer<T> {
        consteval static bool members_have_fixed_size() noexcept {
            return std::apply([&](auto&&... args) { return ((serializer<member_t<args>>::fixed_size) && ...); }, T::fields::value);
        }

        consteval static std::size_t members_size() noexcept {
            return std::apply([&](auto&&... args) { return ((serializer<member_t<args>>::size) + ...); }, T::fields::value);
        }

        constexpr static bool        has_fixed_size = members_have_fixed_size();
        constexpr static std::size_t fixed_size     = has_fixed_size ? members_size() : 0;

        template<bool CheckSize = not has_fixed_size> constexpr static std::span<std::byte> serialize(T const& v, std::span<std::byte> buffer, serializer_options o) noexcept {
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

    /// @brief Type used to describe the members of a composite type
    /// @tparam ...Ts list of member pointers of given type
    template<auto... Ts> struct serializable_fields {
        constexpr static auto value = std::make_tuple(Ts...);
    };


    template<typename T>
    // requires(requires { serializer<T>::serialize; })
    constexpr auto serialize(T const& v, std::span<std::byte> ar, serializer_options o = serializer_options::none) noexcept {
        auto remaining = serializer<T>::template serialize<true>(v, ar, o);
        return remaining;
        // return (remaining.data() != nullptr) ? ar.first(ar.size() - remaining.size()) : std::span<std::byte>{};
    }

} // namespace ecpp

#endif
