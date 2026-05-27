#pragma once

#include <algorithm>
#include <array>
#include <cmath>
#include <cstdint>
#include <ostream>

#include "can/signal.h"

namespace messages {

inline std::uint64_t scaled_to_raw(double value,
                                   double scale,
                                   double offset,
                                   std::uint64_t max_raw) {
    auto raw = static_cast<long long>(std::llround((value - offset) / scale));

    if (raw < 0) {
        return 0;
    }

    auto unsigned_raw = static_cast<std::uint64_t>(raw);
    return std::min(unsigned_raw, max_raw);
}

inline double raw_to_scaled(std::uint64_t raw, double scale, double offset) {
    return static_cast<double>(raw) * scale + offset;
}

// Message 0x100 is vehicle speed in kilometers per hour.
// Signal layout:
//   bits 0..15 = speed_kph / 0.01
struct VehicleSpeed {
    static constexpr std::uint32_t id = 0x100;

    double speed_kph{};

    static VehicleSpeed decode(std::array<std::uint8_t, 8> const& data) {
        auto raw = can::extract(data, can::signal<0, 16>{});
        return VehicleSpeed{raw_to_scaled(raw, 0.01, 0.0)};
    }

    std::array<std::uint8_t, 8> encode() const {
        std::array<std::uint8_t, 8> data{};
        auto raw = scaled_to_raw(speed_kph, 0.01, 0.0, 0xffff);
        can::insert(data, raw, can::signal<0, 16>{});
        return data;
    }
};

// Message 0x101 is steering angle in degrees.
// Signal layout:
//   bits 0..15 = signed angle offset by 32768, scale 0.1 deg
struct SteeringAngle {
    static constexpr std::uint32_t id = 0x101;

    double angle_deg{};

    static SteeringAngle decode(std::array<std::uint8_t, 8> const& data) {
        auto raw = can::extract(data, can::signal<0, 16>{});
        auto signed_raw = static_cast<int>(raw) - 32768;
        return SteeringAngle{static_cast<double>(signed_raw) * 0.1};
    }

    std::array<std::uint8_t, 8> encode() const {
        std::array<std::uint8_t, 8> data{};

        auto signed_raw = static_cast<int>(std::llround(angle_deg / 0.1));
        auto raw = std::clamp(signed_raw + 32768, 0, 65535);

        can::insert(data, static_cast<std::uint64_t>(raw), can::signal<0, 16>{});
        return data;
    }
};

// Message 0x102 is brake pressure in kPa.
// Signal layout:
//   bits 0..15 = pressure_kpa / 0.1
struct BrakePressure {
    static constexpr std::uint32_t id = 0x102;

    double pressure_kpa{};

    static BrakePressure decode(std::array<std::uint8_t, 8> const& data) {
        auto raw = can::extract(data, can::signal<0, 16>{});
        return BrakePressure{raw_to_scaled(raw, 0.1, 0.0)};
    }

    std::array<std::uint8_t, 8> encode() const {
        std::array<std::uint8_t, 8> data{};
        auto raw = scaled_to_raw(pressure_kpa, 0.1, 0.0, 0xffff);
        can::insert(data, raw, can::signal<0, 16>{});
        return data;
    }
};

// Message 0x103 is battery temperature in degrees Celsius.
// Signal layout:
//   bits 0..7 = temperature_c + 40
struct BatteryTemperature {
    static constexpr std::uint32_t id = 0x103;

    double temperature_c{};

    static BatteryTemperature decode(std::array<std::uint8_t, 8> const& data) {
        auto raw = can::extract(data, can::signal<0, 8>{});
        return BatteryTemperature{raw_to_scaled(raw, 1.0, -40.0)};
    }

    std::array<std::uint8_t, 8> encode() const {
        std::array<std::uint8_t, 8> data{};
        auto raw = scaled_to_raw(temperature_c, 1.0, -40.0, 0xff);
        can::insert(data, raw, can::signal<0, 8>{});
        return data;
    }
};

// Message 0x104 is 4 wheel speeds packed into one frame.
// Signal layout:
//   bits 0..11   = front_left_kph / 0.1
//   bits 12..23  = front_right_kph / 0.1
//   bits 24..35  = rear_left_kph / 0.1
//   bits 36..47  = rear_right_kph / 0.1
struct WheelSpeeds {
    static constexpr std::uint32_t id = 0x104;

    double front_left_kph{};
    double front_right_kph{};
    double rear_left_kph{};
    double rear_right_kph{};

    static WheelSpeeds decode(std::array<std::uint8_t, 8> const& data) {
        return WheelSpeeds{
            .front_left_kph = raw_to_scaled(can::extract(data, can::signal<0, 12>{}), 0.1, 0.0),
            .front_right_kph = raw_to_scaled(can::extract(data, can::signal<12, 12>{}), 0.1, 0.0),
            .rear_left_kph = raw_to_scaled(can::extract(data, can::signal<24, 12>{}), 0.1, 0.0),
            .rear_right_kph = raw_to_scaled(can::extract(data, can::signal<36, 12>{}), 0.1, 0.0)
        };
    }

    std::array<std::uint8_t, 8> encode() const {
        std::array<std::uint8_t, 8> data{};

        can::insert(data, scaled_to_raw(front_left_kph, 0.1, 0.0, 0xfff), can::signal<0, 12>{});
        can::insert(data, scaled_to_raw(front_right_kph, 0.1, 0.0, 0xfff), can::signal<12, 12>{});
        can::insert(data, scaled_to_raw(rear_left_kph, 0.1, 0.0, 0xfff), can::signal<24, 12>{});
        can::insert(data, scaled_to_raw(rear_right_kph, 0.1, 0.0, 0xfff), can::signal<36, 12>{});

        return data;
    }
};

inline std::ostream& operator<<(std::ostream& os, VehicleSpeed const& msg) {
    return os << "VehicleSpeed{speed_kph=" << msg.speed_kph << "}";
}

inline std::ostream& operator<<(std::ostream& os, SteeringAngle const& msg) {
    return os << "SteeringAngle{angle_deg=" << msg.angle_deg << "}";
}

inline std::ostream& operator<<(std::ostream& os, BrakePressure const& msg) {
    return os << "BrakePressure{pressure_kpa=" << msg.pressure_kpa << "}";
}

inline std::ostream& operator<<(std::ostream& os, BatteryTemperature const& msg) {
    return os << "BatteryTemperature{temperature_c=" << msg.temperature_c << "}";
}

inline std::ostream& operator<<(std::ostream& os, WheelSpeeds const& msg) {
    return os
        << "WheelSpeeds{front_left_kph=" << msg.front_left_kph
        << ", front_right_kph=" << msg.front_right_kph
        << ", rear_left_kph=" << msg.rear_left_kph
        << ", rear_right_kph=" << msg.rear_right_kph
        << "}";
}

}