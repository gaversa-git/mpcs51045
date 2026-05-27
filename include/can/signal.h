#pragma once

#include <array>
#include <cstddef>
#include <cstdint>
#include <limits>

namespace can {

// A signal describes a packed field inside the 8-byte CAN payload.
// StartBit and Width are compile-time constants, intentionally so that invalid
// bit layouts are rejected by static_assert instead of becoming runtime bugs.
template<std::size_t StartBit, std::size_t Width>
struct signal {
    static_assert(Width > 0, "signal width must be positive");
    static_assert(Width <= 64, "signal width cannot exceed 64 bits");
    static_assert(StartBit + Width <= 64, "signal must fit in an 8-byte payload");

    static constexpr std::size_t start_bit = StartBit;
    static constexpr std::size_t width = Width;
};

// Pack the byte array into a uint64_t using little-endian byte order.
constexpr std::uint64_t pack_little_endian(std::array<std::uint8_t, 8> const& data) {
    std::uint64_t raw = 0;

    for (std::size_t i = 0; i < 8; ++i) {
        raw |= static_cast<std::uint64_t>(data[i]) << (8 * i);
    }

    return raw;
}

// Unpack a uint64_t back into an 8-byte payload.
constexpr std::array<std::uint8_t, 8> unpack_little_endian(std::uint64_t raw) {
    std::array<std::uint8_t, 8> data{};

    for (std::size_t i = 0; i < 8; ++i) {
        data[i] = static_cast<std::uint8_t>((raw >> (8 * i)) & 0xff);
    }

    return data;
}

// Extract an unsigned integer from a little-endian signal layout.
template<std::size_t StartBit, std::size_t Width>
constexpr std::uint64_t extract(std::array<std::uint8_t, 8> const& data,
                                signal<StartBit, Width> = {}) {
    std::uint64_t raw = pack_little_endian(data);

    if constexpr (Width == 64) {
        return raw;
    } else {
        std::uint64_t mask = (std::uint64_t{1} << Width) - 1;
        return (raw >> StartBit) & mask;
    }
}

// Insert an unsigned integer into a little-endian signal layout.
template<std::size_t StartBit, std::size_t Width>
constexpr void insert(std::array<std::uint8_t, 8>& data,
                      std::uint64_t value,
                      signal<StartBit, Width> = {}) {
    std::uint64_t raw = pack_little_endian(data);

    std::uint64_t mask = Width == 64
        ? std::numeric_limits<std::uint64_t>::max()
        : ((std::uint64_t{1} << Width) - 1);

    raw &= ~(mask << StartBit);
    raw |= (value & mask) << StartBit;

    data = unpack_little_endian(raw);
}

}