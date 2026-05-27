#pragma once

#include <cstdint>
#include <optional>
#include <variant>

#include "can/frame.h"
#include "can/message_concept.h"

namespace can {

// Compile-time registry of known CAN messages. Replaces a large manual switch
// on frame IDs. The registry owns the list of message types at compile time,
// then returns a variant containing whichever concrete message type matched the
// incoming frame.
template<message... Messages>
class message_registry {
public:
    static_assert(sizeof...(Messages) > 0, "message_registry needs at least one message type");

    using variant_type = std::variant<Messages...>;
    using decode_result = std::optional<variant_type>;

    static constexpr std::size_t size = sizeof...(Messages);

    static decode_result decode(frame const& f) {
        return decode_impl<Messages...>(f);
    }

    static constexpr bool contains_id(std::uint32_t id) {
        return ((Messages::id == id) || ...);
    }

private:
    template<message First, message... Rest>
    static decode_result decode_impl(frame const& f) {
        if (f.id == First::id) {
            return variant_type{First::decode(f.data)};
        }

        if constexpr (sizeof...(Rest) > 0) {
            return decode_impl<Rest...>(f);
        } else {
            return std::nullopt;
        }
    }
};

}