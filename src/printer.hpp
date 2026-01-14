#pragma once

#include <iostream>
#include <llvm/Support/raw_ostream.h>

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

    std::streampos current_pos() const { return output_stream.tellp(); }
};

template <typename T> stream_proxy& stream_proxy::operator<<(const T& value) {
    if(enabled_ref) {
        output_stream << value;
    }
    return *this;
}

class llvm_stream_proxy : private stream_proxy, public llvm::raw_ostream {
    public:
    explicit llvm_stream_proxy(const stream_proxy& proxy)
    : stream_proxy(proxy) {}

    private:
    void write_impl(const char* Ptr, size_t Size) override {
        stream_proxy::operator<<(std::string{ Ptr, Size });
    };

    uint64_t current_pos() const override {
        return stream_proxy::current_pos();
    }
};

class printer {
    public:
    printer(std::ostream& os)
    : code(os, print_code), debug(os, print_debug), alloc(os, print_alloc) {}

    bool print_code{ true };
    bool print_debug{ true };
    bool print_alloc{ true };

    stream_proxy code;
    stream_proxy debug;
    stream_proxy alloc;
};


} // namespace func
