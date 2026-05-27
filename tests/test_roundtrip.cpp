#include <array>
#include <cassert>
#include <cmath>
#include <cstddef>
#include <iostream>
#include <memory_resource>
#include <sstream>
#include <string>
#include <unordered_map>
#include <variant>
#include <vector>

#include "can/batch_decode.h"
#include "can/codec.h"
#include "can/dump.h"
#include "can/hash.h"
#include "can/message_concept.h"
#include "can/pmr_buffer.h"
#include "can/registry.h"
#include "can/static_dispatch.h"
#include "can/typelist.h"
#include "vehicle_messages.h"

struct NotAMessage {
    int value{};
};

static_assert(!can::message<NotAMessage>);

using Network = can::message_registry<
    messages::VehicleSpeed,
    messages::SteeringAngle,
    messages::BrakePressure,
    messages::BatteryTemperature,
    messages::WheelSpeeds
>;

using MessageTypes = can::typelist<
    messages::VehicleSpeed,
    messages::SteeringAngle,
    messages::BrakePressure,
    messages::BatteryTemperature,
    messages::WheelSpeeds
>;

static_assert(can::length_v<MessageTypes> == 5);
static_assert(can::contains_v<MessageTypes, messages::VehicleSpeed>);
static_assert(!can::contains_v<MessageTypes, NotAMessage>);

struct CountingHandler : can::handler_base<CountingHandler> {
    int speed_count{};
    int steering_count{};
    int brake_count{};
    int battery_count{};
    int wheel_speed_count{};

    void on_message(messages::VehicleSpeed const&) {
        ++speed_count;
    }

    void on_message(messages::SteeringAngle const&) {
        ++steering_count;
    }

    void on_message(messages::BrakePressure const&) {
        ++brake_count;
    }

    void on_message(messages::BatteryTemperature const&) {
        ++battery_count;
    }

    void on_message(messages::WheelSpeeds const&) {
        ++wheel_speed_count;
    }
};

int main() {
    static_assert(Network::contains_id(messages::VehicleSpeed::id));
    static_assert(Network::contains_id(messages::SteeringAngle::id));
    static_assert(Network::contains_id(messages::BrakePressure::id));
    static_assert(Network::contains_id(messages::BatteryTemperature::id));
    static_assert(Network::contains_id(messages::WheelSpeeds::id));
    static_assert(!Network::contains_id(0x777));

    {
        auto original = messages::VehicleSpeed{123.45};
        auto f = can::encode(original);
        auto decoded = can::decode_as<messages::VehicleSpeed>(f);

        assert(std::abs(decoded.speed_kph - original.speed_kph) < 0.02);
    }

    {
        auto original = messages::SteeringAngle{-24.6};
        auto f = can::encode(original);
        auto decoded = can::decode_as<messages::SteeringAngle>(f);

        assert(std::abs(decoded.angle_deg - original.angle_deg) < 0.2);
    }

    {
        auto original = messages::BrakePressure{981.2};
        auto f = can::encode(original);
        auto decoded = can::decode_as<messages::BrakePressure>(f);

        assert(std::abs(decoded.pressure_kpa - original.pressure_kpa) < 0.2);
    }

    {
        auto original = messages::BatteryTemperature{41.0};
        auto f = can::encode(original);
        auto decoded = can::decode_as<messages::BatteryTemperature>(f);

        assert(std::abs(decoded.temperature_c - original.temperature_c) < 1.1);
    }

    {
        auto original = messages::WheelSpeeds{55.1, 55.2, 55.3, 55.4};
        auto f = can::encode(original);
        auto decoded = can::decode_as<messages::WheelSpeeds>(f);

        assert(std::abs(decoded.front_left_kph - original.front_left_kph) < 0.2);
        assert(std::abs(decoded.front_right_kph - original.front_right_kph) < 0.2);
        assert(std::abs(decoded.rear_left_kph - original.rear_left_kph) < 0.2);
        assert(std::abs(decoded.rear_right_kph - original.rear_right_kph) < 0.2);
    }

    {
        auto f = can::encode(messages::VehicleSpeed{44.4});
        auto decoded = Network::decode(f);

        assert(decoded.has_value());
        assert(std::holds_alternative<messages::VehicleSpeed>(*decoded));

        auto const& msg = std::get<messages::VehicleSpeed>(*decoded);
        assert(std::abs(msg.speed_kph - 44.4) < 0.02);
    }

    {
        can::frame unknown{.id = 0x777, .data = {}, .dlc = 8};
        auto decoded = Network::decode(unknown);

        assert(!decoded.has_value());
    }

    {
        auto f1 = can::encode(messages::VehicleSpeed{10.0});
        auto f2 = can::encode(messages::VehicleSpeed{10.0});
        auto f3 = can::encode(messages::VehicleSpeed{20.0});

        assert(f1 == f2);
        assert(!(f1 == f3));

        std::unordered_map<can::frame, std::string> labels;
        labels[f1] = "first speed frame";
        labels[f3] = "second speed frame";

        assert(labels.at(f2) == "first speed frame");
        assert(labels.at(f3) == "second speed frame");
    }

    {
        auto f = can::encode(messages::SteeringAngle{-3.2});
        auto line = can::to_log_line(f);
        auto parsed = can::parse_log_line(line);

        assert(parsed.has_value());
        assert(parsed->id == f.id);
        assert(parsed->data == f.data);
        assert(parsed->dlc == f.dlc);
    }

    {
        std::stringstream input;
        input << can::to_log_line(can::encode(messages::VehicleSpeed{10.0})) << "\n";
        input << can::to_log_line(can::encode(messages::SteeringAngle{2.0})) << "\n";
        input << "777#0000000000000000\n";

        auto frames = can::read_log(input);
        assert(frames.size() == 3);

        auto decoded = can::decode_all<Network>(frames);
        assert(decoded.size() == 3);

        assert(decoded[0].has_value());
        assert(std::holds_alternative<messages::VehicleSpeed>(*decoded[0]));

        assert(decoded[1].has_value());
        assert(std::holds_alternative<messages::SteeringAngle>(*decoded[1]));

        assert(!decoded[2].has_value());
    }

    {
        std::vector<can::frame> frames{
            can::encode(messages::VehicleSpeed{1.0}),
            can::encode(messages::SteeringAngle{2.0}),
            can::encode(messages::BrakePressure{3.0}),
            can::encode(messages::BatteryTemperature{4.0}),
            can::encode(messages::WheelSpeeds{5.0, 6.0, 7.0, 8.0})
        };

        auto decoded = can::decode_all<Network>(frames);

        CountingHandler handler;
        for (auto const& item : decoded) {
            can::dispatch_to(handler, item);
        }

        assert(handler.speed_count == 1);
        assert(handler.steering_count == 1);
        assert(handler.brake_count == 1);
        assert(handler.battery_count == 1);
        assert(handler.wheel_speed_count == 1);
    }

    {
        std::array<std::byte, 4096> storage{};
        std::pmr::monotonic_buffer_resource resource{storage.data(), storage.size()};
        can::pmr_frame_buffer buffer{&resource};

        buffer.push(can::encode(messages::VehicleSpeed{11.0}));
        buffer.push(can::encode(messages::SteeringAngle{12.0}));

        assert(buffer.size() == 2);
        assert(buffer.frames()[0].id == messages::VehicleSpeed::id);
        assert(buffer.frames()[1].id == messages::SteeringAngle::id);
    }

    std::cout << "Full roundtrip tests passed\n";
}