#ifndef DATA_DYN_STRING_HPP
#define DATA_DYN_STRING_HPP

#include "data/String.hpp"
#include "sys/mem.hpp"

#include <memory>

class DynString
{
    const std::unique_ptr<const char[]> _buffer;
    const size_t _size;

public:
    DynString() : _buffer(), _size(0) {}

    DynString(const char *str, size_t n) : _buffer(mem::dup(str, n)), _size(n)
    {}

    DynString(const String &s) : DynString(s.data(), s.size()) {}

    template<size_t N>
    explicit DynString(const char (&s)[N]) : DynString(s, N - 1)
    {}

    size_t size() const { return _size; }
    const char *data() const { return _buffer.get(); }

    operator String() const { return String(data(), size()); }

    char operator[](size_t i) const { return data()[i]; }
};

#endif
