#pragma once

#include <algorithm>
#include <vector>

#include "can/frame.h"

namespace can {

// Decode a batch of frames through a registry, intended for offline log
// processing.

template<typename Registry>
auto decode_all(std::vector<frame> const& frames) {
    using result_type = typename Registry::decode_result;

    std::vector<result_type> results;
    results.resize(frames.size());

    std::transform(frames.begin(),
                   frames.end(),
                   results.begin(),
                   [](frame const& f) {
                       return Registry::decode(f);
                   });

    return results;
}

}