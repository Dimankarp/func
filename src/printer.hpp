#pragma once

#include <iostream>
#include <string>

namespace cmplr {

class stream_proxy {
private:
    std::ostream& output_stream;
    bool& enabled_ref;
    
public:
    stream_proxy(std::ostream& os, bool& enabled);
    
    template<typename T>
    stream_proxy& operator<<(const T& value);
    
    stream_proxy& operator<<(std::ostream& (*manip)(std::ostream&));
};


class printer {

public:
    printer(std::ostream& os = std::cout);
    
    bool print_code;
    bool print_debug;
    bool print_alloc;
    
    stream_proxy code;
    stream_proxy debug;
    stream_proxy alloc;

private:
    std::ostream& output_stream;
};

template<typename T>
stream_proxy& stream_proxy::operator<<(const T& value) {
    if (enabled_ref) {
        output_stream << value;
    }
    return *this;
}

} // namespace cmplr
