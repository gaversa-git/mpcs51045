#pragma once

#include <array>
#include <concepts>
#include <cstdint>

namespace can {

// A CAN message type must expose: static constexpr id, static decode(data),
// encode() const
// This gives template code a clean contract without using inheritance.
template<typename T>
concept message = requires(T msg, std::array<std::uint8_t, 8> data) {
    { T::id } -> std::convertible_to<std::uint32_t>;
    { T::decode(data) } -> std::same_as<T>;
    { msg.encode() } -> std::same_as<std::array<std::uint8_t, 8>>;
};

}
