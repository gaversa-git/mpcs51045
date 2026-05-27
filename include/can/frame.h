#pragma once

#include <array>
#include <cstddef>
#include <cstdint>
#include <compare>
#include <stdexcept>

namespace can {

// Raw CAN frame representation.
struct frame {
    std::uint32_t id{};
    std::array<std::uint8_t, 8> data{};
    std::size_t dlc{8};

    // Allow compiler to gen code for all 6 comparison ops
    auto operator<=>(frame const&) const = default;
};

// Helper for safely constructing a frame.
// This provides one place to validate DLC rather than scattering checks.
inline frame make_frame(std::uint32_t id, std::array<std::uint8_t, 8> data, std::size_t dlc = 8) {
    if (dlc > 8) {
        throw std::invalid_argument("A classic CAN dlc cannot exceed 8");
    }
    return frame{id, data, dlc};
}

}