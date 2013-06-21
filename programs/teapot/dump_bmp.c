#include "dump_bmp.h"

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

typedef struct
{
    uint16_t magic;
    uint32_t byte_size;
    uint32_t reserved;
    uint32_t payload_byte_offset;
} __attribute__((packed)) BMPHeader;

typedef struct
{
    uint32_t info_byte_size;
    int32_t  pixel_width;
    int32_t  pixel_height;
    uint16_t planes;
    uint16_t bpp;
    uint32_t compression_type;
    uint32_t payload_byte_size;
    int32_t  pixelsX_per_meter;
    int32_t  pixelsY_per_meter;
    uint32_t color_table_size;
    uint32_t used_colors;
} __attribute__((packed)) BMPInfo;

typedef struct {
    BMPHeader header;
    BMPInfo info;
} __attribute__((packed)) BMPPrefix;
   
typedef struct {
    unsigned char b, g, r;
} __attribute__((packed)) Pixel24;

static const uint16_t MAGIC = 19778; // ascii "BM"

static const unsigned BUF_SIZE = 1024;

// bmp uses little endian encoding as does x86,
// so the encoding/decoding routines dont do anything

static uint16_t encodeU16(uint16_t x)
{
    return x;
}

static uint32_t encodeU32(uint32_t x)
{
    return x;
}

static int32_t encode32(int32_t x)
{
    return x;
}

static uint16_t decodeU16(uint16_t x)
{
    return x;
}

static uint32_t decodeU32(uint32_t x)
{
    return x;
}

static int32_t decode32(int32_t x)
{
    return x;
}


static void convert_pixels(const uint32_t *src, Pixel24 *dst, unsigned size)
{
    unsigned i;
    for (i = 0; i < size; ++i)
    {
        uint32_t p = src[i];
        Pixel24 out;
        out.r = p & 0xFF;
        out.g = (p >> 8) & 0xFF;
        out.b = (p >> 16) & 0xFF;
        dst[i] = out;
    }
}

BMPError bmp_dump(int fd, int w, int h, const uint32_t *pixels)
{
    
    BMPHeader header = {
        .magic               = encodeU16(MAGIC),
        .byte_size           = encodeU32(0),
        .reserved            = encodeU32(0),
        .payload_byte_offset = encodeU32(sizeof (BMPHeader) + sizeof (BMPInfo))
    };

    BMPInfo info = {
        .info_byte_size    = encodeU32(40),
        .pixel_width       = encode32(w),
        .pixel_height      = encode32(h),
        .planes            = encodeU16(1),
        .bpp               = encodeU16(24),
        .compression_type  = encodeU32(0),
        .payload_byte_size = encodeU32(0),
        .pixelsX_per_meter = encode32(0),
        .pixelsY_per_meter = encode32(0),
        .color_table_size  = encodeU32(0),
        .used_colors       = encodeU32(0)
    };

    BMPPrefix prefix;
    prefix.header = header;
    prefix.info = info;

    if (write(fd, &prefix, sizeof prefix) < 0)
        goto write_err;

    {
        Pixel24 pixel_buf[BUF_SIZE];

        unsigned i;
        for (i = 0; i < w * h / BUF_SIZE; ++i)
        {
            convert_pixels(pixels + i * BUF_SIZE, pixel_buf, BUF_SIZE);
            if (write(fd, pixel_buf, sizeof pixel_buf) < 0)
                goto write_err;
        }

        unsigned offset   = i * BUF_SIZE;
        unsigned rem_size = w * h - offset;
        if (rem_size > 0)
        {
            convert_pixels(pixels + offset, pixel_buf, rem_size);
            if (write(fd, pixel_buf, sizeof *pixel_buf * rem_size) < 0)
                goto write_err;
        }
    }

    return BMP_OK;

write_err:
    perror("error writing bitmap");

    return BMP_IO;
}

BMPError bmp_read(int fd, int *wp, int *hp, uint32_t **pixelsp) {
    uint32_t *pixels = NULL;
    off_t offset;
    int32_t w, h;
    unsigned char *bytes;
    Pixel24 *data;
    int i;

    *wp = 0;
    *hp = 0;
    *pixelsp = NULL;

    BMPPrefix prefix;
    if (read(fd, &prefix, sizeof prefix) != (ssize_t) sizeof prefix)
        goto read_err;

    if (decodeU16(prefix.header.magic) != MAGIC)
        return BMP_FORMAT;

    w = decode32(prefix.info.pixel_width);
    h = decode32(prefix.info.pixel_height);

    if (h < 0 ||
        decodeU16(prefix.info.planes) != 1 ||
        decodeU16(prefix.info.bpp) != 24 ||
        decodeU32(prefix.info.compression_type) != 0 ||
        decodeU32(prefix.info.color_table_size) != 0)
        return BMP_IO;

    offset = decodeU32(prefix.header.payload_byte_offset) - sizeof (BMPPrefix);
    if (offset > 0) {
        if (lseek(fd, offset, SEEK_CUR) == (off_t) -1)
            goto read_err;
    }

    pixels = (uint32_t *) malloc(w * h * sizeof(uint32_t));
    if (!pixels)
        return BMP_IO;

    bytes = (unsigned char *) pixels;
    
    if (read(fd, bytes + w * h, w * h * 3) != w * h * 3)
        goto read_err;

    data = (Pixel24 *)(bytes + w * h);
    for (i = 0; i < w * h; ++i) {
        Pixel24 p = data[i];
        uint32_t rgba = p.r | ((uint32_t) p.g << 8) | ((uint32_t) p.b << 16) | ((uint32_t) 255 << 24);
        pixels[i] = rgba;
    }

    *wp = w;
    *hp = h;
    *pixelsp = pixels;
    
    return BMP_OK;

read_err:
    if (pixels) free(pixels);
    perror("error reading bitmap");
    return BMP_IO;
}
