#include "sys/io.hpp"

#include "err/err.hpp"
#include "sys/module.hpp"

#include <bl/array.hpp>
#include <bl/optional.hpp>
#include <bl/tuple.hpp>

#include <cstring>

namespace sys::io {

using namespace bl::literals;

IO::IO() : ipa_any(0), ipa_local(127, 0, 0, 1) {}

IOResult<Array<bl::cbyte>>
readFile(bl::string_view path) noexcept
{
    static constexpr auto BUF_SIZE = 8_KiB;
    auto res = open(path, HM_READ);
    auto err = IOError{};
    if (!res) {
        err = res.error();
        goto fail;
    }
    {
        auto h = bl::move(res).value();
        ByteStream str;
        for (;;) {
            bl::array<bl::byte, BUF_SIZE> buf{};
            auto size = 0_z;
            bl::tie(size, err) = read(h, bl::span{ buf });
            if (size > 0)
                write(str, bl::span{ buf }.first(size));
            if (err == IOError::EOF) {
                err = IOError::OK;
                return { { str.data(), str.size() } };
            }
            if (err != IOError::OK)
                goto fail;
        }
    }
fail:
    stderr() << "unable to read file: "_k << path << "\n"_k;

    return util::unexpected{ err };
}

} // namespace sys::io
