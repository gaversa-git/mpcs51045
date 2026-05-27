#pragma once

#include <stdexcept>

#include "can/frame.h"
#include "can/message_concept.h"

namespace can {

// Decodde a raw frame into a specific message type.
// This is constrained so it only accepts valid CAN message types.
template<message Message>
Message decode_as(frame const& f) {
    if (f.id != Message::id) {
        throw std::invalid_argument("frame id does not match requested message type");
    }
    return Message::decode(f.data);
}

// Encode a typed message into a raw CAN frame.
template<message Message>
frame encode(Message const& msg) {
    return frame{
        .id = Message::id,
        .data = msg.encode(),
        .dlc = 8
    };
}

}