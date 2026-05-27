#pragma once

#include <cstddef>
#include <cstdint>
#include <string>
#include <variant>

namespace can {

// Error type for an unknown frame ID.
struct unknown_message_id {
    std::uint32_t id{};
};

// Error type for malformed or unexpected frame length.
struct invalid_dlc {
    std::size_t actual{};
    std::size_t expected{};
};

// Error type for a malformed textual log line.
struct parse_error {
    std::string line;
    std::string reason;
};

using decode_error = std::variant<unknown_message_id, invalid_dlc, parse_error>;

}