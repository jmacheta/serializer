#ifndef ECPP_SERIAL_UTILS_HPP_
#define ECPP_SERIAL_UTILS_HPP_

namespace ecpp {
namespace serial_impl {

template <typename T>
struct member_type_from_pointer;

template <class T, class U>
struct member_type_from_pointer<T U::*> : std::type_identity<T> {};

template <typename T>
concept fundamental = std::is_fundamental_v<std::remove_cvref_t<T>>;

template <typename T>
concept has_field_descr = requires { typename T::fields; };

template <typename T>
concept array = std::is_array_v<T>;

}  // namespace serial_impl
}  // namespace ecpp
#endif