#ifndef DATA_STRING_HPP
#define DATA_STRING_HPP

#include "defs.hpp"

class String
{
    const char *_buffer;
    const size_t _size;

public:
    constexpr String() : _buffer(nullptr), _size(0) {}

    template<size_t N>
    explicit constexpr String(const char (&s)[N]) : String(s, N - 1)
    {}

    constexpr String(const char *s, size_t n) : _buffer(s), _size(n) {}

    constexpr size_t size() const { return _size; }
    constexpr const char *data() const { return _buffer; }

    constexpr char operator[](size_t i) const { return _buffer[i]; }
};

#endif
