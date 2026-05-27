#pragma once

namespace can {

// Helper for building overloaded visitors for std::variant.
template<class... Fs>
struct overloaded : Fs... {
    using Fs::operator()...;
};

template<class... Fs>
overloaded(Fs...) -> overloaded<Fs...>;

}