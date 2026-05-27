#pragma once

#include <optional>
#include <variant>

#include "can/message_concept.h"

namespace can {

template<typename Derived>
struct handler_base {
    template<message Message>
    void handle(Message const& msg) {
        static_cast<Derived&>(*this).on_message(msg);
    }
};

// Dispatch an optional variant result into a handler. Note: Unknown frames are
// ignored here.
template<typename Handler, typename Variant>
void dispatch_to(Handler& handler, std::optional<Variant> const& decoded) {
    if (!decoded) {
        return;
    }

    std::visit(
        [&](auto const& msg) {
            handler.handle(msg);
        },
        *decoded
    );
}

}