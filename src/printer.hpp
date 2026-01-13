#pragma once

#include <iostream>

namespace func {

class stream_proxy {
    private:
    std::ostream& output_stream;
    bool& enabled_ref;

    public:
    stream_proxy(std::ostream& os, bool& enabled)
    : output_stream(os), enabled_ref(enabled) {}

    template <typename T> stream_proxy& operator<<(const T& value);

    stream_proxy& operator<<(std::ostream& (*manip)(std::ostream&)) {
        if(enabled_ref) {
            output_stream << manip;
        }
        return *this;
    }
};

class printer {
    public:
    printer(std::ostream& os)
    : code(os, print_code), debug(os, print_debug), alloc(os, print_alloc){}

    bool print_code{ true };
    bool print_debug{ true };
    bool print_alloc{ true };

    stream_proxy code;
    stream_proxy debug;
    stream_proxy alloc;

};

template <typename T> stream_proxy& stream_proxy::operator<<(const T& value) {
    if(enabled_ref) {
        output_stream << value;
    }
    return *this;
}

} // namespace func
