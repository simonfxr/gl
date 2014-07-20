#include "sys/io.hpp"
#include "err/err.hpp"
#include "sys/module.hpp"

#include <string.h>

namespace sys {

namespace io {

namespace {

StreamResult convertErr(HandleError err) {
    switch (err) {
    case HE_OK: return StreamResult::OK;
    case HE_BLOCKED: return StreamResult::Blocked;
    case HE_EOF: return StreamResult::EOF;
    case HE_BAD_HANDLE:
    case HE_INVALID_PARAM:
    case HE_UNKNOWN:
        return StreamResult::Error;
    }

    ASSERT_FAIL();
}

} // namespace anon

IO::IO() :
    ipa_any(0),
    ipa_local(127, 0, 0, 1)
{}

const IPAddr4 IPA_ANY() {
    return module->io.ipa_any;
}

const IPAddr4 IPA_LOCAL() {
    return module->io.ipa_local;
}

HandleStream::HandleStream(const Handle& h) :
    handle(h),
    read_cursor(0),
    write_cursor(0)
{}

HandleStream::~HandleStream() {
    close();
}

StreamResult HandleStream::basic_close() {
    StreamResult ret = basic_flush();
    sys::io::close(handle);
    return ret;
}

StreamResult HandleStream::basic_flush() {
    if (write_cursor > 0)
        return flush_buffer();
    else
        return StreamResult::OK;
}

StreamResult HandleStream::basic_read(size& s, char *buf) {
    if (s == 0)
        return StreamResult::OK;
    
    if (s <= read_cursor) {
        memcpy(buf, read_buffer, UNSIZE(s));
        read_cursor -= s;
        if (read_cursor > 0)
            memmove(read_buffer, read_buffer + s, read_cursor);

        ASSERT(s > 0);
        return StreamResult::OK;
    }

    size n = read_cursor;
    memcpy(buf, read_buffer, read_cursor);
    s -= read_cursor;
    buf += read_cursor;
    HandleError err;

    if (s > HANDLE_READ_BUFFER_SIZE) {
        size k = s;
        err = sys::io::read(handle, k, buf);
        read_cursor = 0;
        n += k;        
    } else {
        size k = HANDLE_READ_BUFFER_SIZE;
        err = sys::io::read(handle, k, read_buffer);
        if (k > s) {
            n += s;
            read_cursor = k - s;
            memcpy(buf, read_buffer, s);
            memmove(read_buffer, read_buffer + s, k - s);
            
            if (err == HE_BLOCKED ||
                err == HE_EOF)
                err = HE_OK;
        } else {
            read_cursor = 0;
            n += k;
            memcpy(buf, read_buffer, k);
        }
    }

    s = n;
    StreamResult res = convertErr(err);
    ASSERT(res != StreamResult::OK || s > 0);
    return res;
}

StreamResult HandleStream::basic_write(size& s, const char *buf) {
    size rem = HANDLE_WRITE_BUFFER_SIZE - write_cursor;
    
    if (s <= rem) {
        memcpy(write_buffer + write_cursor, buf, s);
        write_cursor += s;
        return StreamResult::OK;
    }

    size n = 0;
    size k = 0;
    HandleError err = HE_OK;

    if (write_cursor > 0) {
        memcpy(write_buffer + write_cursor, buf, rem);
        k = HANDLE_WRITE_BUFFER_SIZE;
        err = sys::io::write(handle, k, write_buffer);
    }
        
    if (write_cursor == 0 ||
        (k == HANDLE_WRITE_BUFFER_SIZE && err == HE_OK)) {

        n += rem;
        k = s - rem;
        err = sys::io::write(handle, k, buf + rem);
        n += k;
        const char *unwritten = buf + n;
        size rest = s - n;
        if (rest > 0) {
            size into = rest <= HANDLE_WRITE_BUFFER_SIZE ? rest : HANDLE_WRITE_BUFFER_SIZE;
            memcpy(write_buffer, unwritten, into);
            n += into;
            write_cursor = into;
        } else {
            write_cursor = 0;
        }
    } else {
        if (k > 0) {
            size end = HANDLE_WRITE_BUFFER_SIZE - k;
            size rest = k <= write_cursor ? s : s - (k - write_cursor);
            size into = rest <= k ? rest : k;
                
            memmove(write_buffer, write_buffer + k, end);
            memcpy(write_buffer + end, buf + rem, into);
            n += rem + into;
            write_cursor = end + into;
        } else {
            write_cursor = HANDLE_WRITE_BUFFER_SIZE;
        }
    }

    if (n == s &&
        (err == HE_BLOCKED ||
         err == HE_EOF))
        err = HE_OK;

    s = n;
    return convertErr(err);
}

StreamResult HandleStream::flush_buffer() {
    size k = write_cursor;
    HandleError err = sys::io::write(handle, k, write_buffer);
    
    if (k > 0) {
        memmove(write_buffer, write_buffer + k, write_cursor - k);
        write_cursor -= k;
    }

    return convertErr(err);
}

bool readFile(sys::io::OutStream& err, const std::string& path, char **file_contents, size *file_size) {
    FILE *in = fopen(path.c_str(), "r");
    if (in == 0)
        goto fail;
    
    *file_contents = nullptr;
    *file_size = 0;
    
    if (fseek(in, 0, SEEK_END) == -1)
        goto fail;

    {
        int64 ssize = ftell(in);
        if (ssize < 0)
            goto fail;
        size_t size = size_t(ssize);
        if (fseek(in, 0, SEEK_SET) == -1)
            goto fail;

        {
            char *contents = new char[size + 1];
            if (fread(contents, size, 1, in) != 1) {
                delete[] contents;
                goto fail;
            }
        
            contents[size] = '\0';
            *file_contents = contents;
            *file_size = SIZE(size);
        }
    }

    fclose(in);
    return true;

fail:
    if (err.writeable())
        err << "unable to read file: " << path << sys::io::endl;

    if (in)
        fclose(in);
    return false;
}

} // namespace io

} // namespace sys
