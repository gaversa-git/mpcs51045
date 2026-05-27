#pragma once

#include <cstddef>
#include <cstdint>
#include <functional>

#include "can/frame.h"

namespace can {

inline void hash_combine(std::size_t& seed, std::size_t value) {
    seed ^= value + 0x9e3779b97f4a7c15ULL + (seed << 6) + (seed >> 2);
}

}

namespace std {

template<>
struct hash<can::frame> {
    std::size_t operator()(can::frame const& f) const noexcept {
        std::size_t seed = std::hash<std::uint32_t>{}(f.id);
        can::hash_combine(seed, std::hash<std::size_t>{}(f.dlc));

        for (auto byte : f.data) {
            can::hash_combine(seed, std::hash<unsigned>{}(byte));
        }

        return seed;
    }
};

}