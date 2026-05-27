#pragma once

#include <memory_resource>
#include <vector>

#include "can/frame.h"

namespace can {

class pmr_frame_buffer {
public:
    explicit pmr_frame_buffer(std::pmr::memory_resource* resource)
        : frames_{resource} {}

    void push(frame f) {
        frames_.push_back(f);
    }

    void clear() {
        frames_.clear();
    }

    std::size_t size() const {
        return frames_.size();
    }

    bool empty() const {
        return frames_.empty();
    }

    std::pmr::vector<frame> const& frames() const {
        return frames_;
    }

private:
    std::pmr::vector<frame> frames_;
};

}