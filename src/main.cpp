#include <array>
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
#include "can/pmr_buffer.h"
#include "can/registry.h"
#include "can/static_dispatch.h"
#include "can/visitor.h"
#include "vehicle_messages.h"

using Network = can::message_registry<
    messages::VehicleSpeed,
    messages::SteeringAngle,
    messages::BrakePressure,
    messages::BatteryTemperature,
    messages::WheelSpeeds
>;

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

void print_decoded(Network::decode_result const& decoded) {
    if (!decoded) {
        std::cout << "Unknown frame\n";
        return;
    }

    std::visit(can::overloaded{
        [](messages::VehicleSpeed const& msg) {
            std::cout << "Decoded " << msg << "\n";
        },
        [](messages::SteeringAngle const& msg) {
            std::cout << "Decoded " << msg << "\n";
        },
        [](messages::BrakePressure const& msg) {
            std::cout << "Decoded " << msg << "\n";
        },
        [](messages::BatteryTemperature const& msg) {
            std::cout << "Decoded " << msg << "\n";
        },
        [](messages::WheelSpeeds const& msg) {
            std::cout << "Decoded " << msg << "\n";
        }
    }, *decoded);
}

int main() {
    std::vector<can::frame> frames{
        can::encode(messages::VehicleSpeed{88.25}),
        can::encode(messages::SteeringAngle{-15.2}),
        can::encode(messages::BrakePressure{1240.5}),
        can::encode(messages::BatteryTemperature{36.0}),
        can::encode(messages::WheelSpeeds{88.0, 88.1, 87.9, 88.2}),
        can::frame{.id = 0x777, .data = {}, .dlc = 8}
    };

    std::cout << "Raw frames:\n";
    for (auto const& f : frames) {
        std::cout << "  " << can::to_pretty_string(f)
                  << "  log=" << can::to_log_line(f) << "\n";
    }

    std::cout << "\nRegistry decode:\n";
    for (auto const& f : frames) {
        print_decoded(Network::decode(f));
    }

    std::cout << "\nBatch decode:\n";
    auto decoded_batch = can::decode_all<Network>(frames);
    for (auto const& decoded : decoded_batch) {
        print_decoded(decoded);
    }

    std::cout << "\nLog parse demo:\n";
    std::stringstream log;
    for (auto const& f : frames) {
        log << can::to_log_line(f) << "\n";
    }

    auto parsed_frames = can::read_log(log);
    std::cout << "Parsed " << parsed_frames.size() << " frames from log\n";

    std::cout << "\nCRTP handler demo:\n";
    CountingHandler handler;
    for (auto const& decoded : decoded_batch) {
        can::dispatch_to(handler, decoded);
    }

    std::cout << "VehicleSpeed count: " << handler.speed_count << "\n";
    std::cout << "SteeringAngle count: " << handler.steering_count << "\n";
    std::cout << "BrakePressure count: " << handler.brake_count << "\n";
    std::cout << "BatteryTemperature count: " << handler.battery_count << "\n";
    std::cout << "WheelSpeeds count: " << handler.wheel_speed_count << "\n";

    std::cout << "\nPMR buffer demo:\n";
    std::array<std::byte, 4096> storage{};
    std::pmr::monotonic_buffer_resource resource{storage.data(), storage.size()};
    can::pmr_frame_buffer buffer{&resource};

    for (auto const& f : frames) {
        buffer.push(f);
    }

    std::cout << "Stored " << buffer.size() << " frames in PMR buffer\n";

    std::cout << "\nUnordered map/hash demo:\n";
    std::unordered_map<can::frame, std::string> labels;
    labels[frames[0]] = "speed frame";
    labels[frames[1]] = "steering frame";

    std::cout << "Label for first frame: " << labels.at(frames[0]) << "\n";

    return 0;
}