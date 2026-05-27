#include <cassert>
#include <cmath>
#include <iostream>

#include "can/codec.h"
#include "can/message_concept.h"
#include "vehicle_messages.h"

static_assert(can::message<messages::VehicleSpeed>);
static_assert(can::message<messages::SteeringAngle>);
static_assert(can::message<messages::BrakePressure>);
static_assert(can::message<messages::BatteryTemperature>);
static_assert(can::message<messages::WheelSpeeds>);

int main() {
    {
        messages::VehicleSpeed original{88.25};
        can::frame f = can::encode(original);

        assert(f.id == messages::VehicleSpeed::id);
        assert(f.dlc == 8);

        auto decoded = can::decode_as<messages::VehicleSpeed>(f);
        assert(std::abs(decoded.speed_kph - original.speed_kph) < 0.02);
    }

    {
        messages::SteeringAngle original{-15.2};
        can::frame f = can::encode(original);

        assert(f.id == messages::SteeringAngle::id);

        auto decoded = can::decode_as<messages::SteeringAngle>(f);
        assert(std::abs(decoded.angle_deg - original.angle_deg) < 0.2);
    }

    std::cout << "Test set 1 passed\n";
}