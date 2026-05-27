#pragma once

#include <array>
#include <cctype>
#include <cstdint>
#include <iomanip>
#include <istream>
#include <optional>
#include <sstream>
#include <string>
#include <string_view>
#include <vector>

#include "can/frame.h"

namespace can {

// Convert a raw frame to a compact candump-like line "100#7922000000000000"
inline std::string to_log_line(frame const& f) {
    std::ostringstream oss;

    oss << std::uppercase << std::hex << f.id << "#";

    for (std::size_t i = 0; i < f.dlc; ++i) {
        oss << std::setw(2) << std::setfill('0')
            << static_cast<unsigned>(f.data[i]);
    }

    return oss.str();
}

inline std::string to_pretty_string(frame const& f) {
    std::ostringstream oss;

    oss << "frame{id=0x" << std::hex << std::uppercase << f.id
        << ", dlc=" << std::dec << f.dlc
        << ", data=[";

    for (std::size_t i = 0; i < f.dlc; ++i) {
        if (i != 0) {
            oss << " ";
        }

        oss << std::hex << std::uppercase << std::setw(2) << std::setfill('0')
            << static_cast<unsigned>(f.data[i]);
    }

    oss << "]}";
    return oss.str();
}

inline void trim(std::string_view& text) {
    while (!text.empty() && std::isspace(static_cast<unsigned char>(text.front()))) {
        text.remove_prefix(1);
    }

    while (!text.empty() && std::isspace(static_cast<unsigned char>(text.back()))) {
        text.remove_suffix(1);
    }
}

inline std::optional<unsigned> hex_value(char c) {
    if ('0' <= c && c <= '9') {
        return static_cast<unsigned>(c - '0');
    }

    if ('a' <= c && c <= 'f') {
        return static_cast<unsigned>(10 + c - 'a');
    }

    if ('A' <= c && c <= 'F') {
        return static_cast<unsigned>(10 + c - 'A');
    }

    return std::nullopt;
}

inline std::optional<std::uint32_t> parse_hex_id(std::string_view text) {
    if (text.empty()) {
        return std::nullopt;
    }

    std::uint32_t id = 0;

    for (char c : text) {
        auto value = hex_value(c);
        if (!value) {
            return std::nullopt;
        }

        id = static_cast<std::uint32_t>((id << 4) | *value);
    }

    return id;
}

inline std::optional<std::uint8_t> parse_hex_byte(char high, char low) {
    auto h = hex_value(high);
    auto l = hex_value(low);

    if (!h || !l) {
        return std::nullopt;
    }

    return static_cast<std::uint8_t>((*h << 4) | *l);
}

// Parse the project-specific candump-like line format "ID#DATA"
// ie. 100#7922000000000000 or 101#CC7F000000000000
inline std::optional<frame> parse_log_line(std::string_view line) {
    trim(line);

    if (line.empty() || line.front() == '#') {
        return std::nullopt;
    }

    auto hash_pos = line.find('#');
    if (hash_pos == std::string_view::npos) {
        return std::nullopt;
    }

    auto id_part = line.substr(0, hash_pos);
    auto data_part = line.substr(hash_pos + 1);

    trim(id_part);
    trim(data_part);

    if (data_part.size() % 2 != 0 || data_part.size() > 16) {
        return std::nullopt;
    }

    auto id = parse_hex_id(id_part);
    if (!id) {
        return std::nullopt;
    }

    std::array<std::uint8_t, 8> data{};
    std::size_t dlc = data_part.size() / 2;

    for (std::size_t i = 0; i < dlc; ++i) {
        auto byte = parse_hex_byte(data_part[2 * i], data_part[2 * i + 1]);
        if (!byte) {
            return std::nullopt;
        }

        data[i] = *byte;
    }

    return frame{
        .id = *id,
        .data = data,
        .dlc = dlc
    };
}

inline std::vector<frame> read_log(std::istream& input) {
    std::vector<frame> frames;
    std::string line;

    while (std::getline(input, line)) {
        if (auto f = parse_log_line(line)) {
            frames.push_back(*f);
        }
    }

    return frames;
}

}