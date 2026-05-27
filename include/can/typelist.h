#pragma once

#include <cstddef>
#include <type_traits>

namespace can {

// Lightweight compile-time list of types.
template<typename... Ts>
struct typelist {};

template<typename List>
struct length;

template<typename... Ts>
struct length<typelist<Ts...>> {
    static constexpr std::size_t value = sizeof...(Ts);
};

template<typename List>
inline constexpr std::size_t length_v = length<List>::value;

template<typename List, typename T>
struct contains;

template<typename T>
struct contains<typelist<>, T> : std::false_type {};

template<typename Head, typename... Tail, typename T>
struct contains<typelist<Head, Tail...>, T>
    : std::conditional_t<std::is_same_v<Head, T>,
                         std::true_type,
                         contains<typelist<Tail...>, T>> {};

template<typename List, typename T>
inline constexpr bool contains_v = contains<List, T>::value;

template<typename First, typename Second>
struct append;

template<typename... Ts, typename... Us>
struct append<typelist<Ts...>, typelist<Us...>> {
    using type = typelist<Ts..., Us...>;
};

template<typename First, typename Second>
using append_t = typename append<First, Second>::type;

template<typename List, std::size_t Index>
struct type_at;

template<typename Head, typename... Tail>
struct type_at<typelist<Head, Tail...>, 0> {
    using type = Head;
};

template<typename Head, typename... Tail, std::size_t Index>
struct type_at<typelist<Head, Tail...>, Index>
    : type_at<typelist<Tail...>, Index - 1> {};

template<typename List, std::size_t Index>
using type_at_t = typename type_at<List, Index>::type;

}