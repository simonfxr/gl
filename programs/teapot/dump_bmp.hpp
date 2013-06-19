#ifndef DUMP_BMP_HPP
#define DUMP_BMP_HPP

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

enum BMPError { BMP_OK, BMP_IO, BMP_FORMAT };
typedef enum BMPError BMPError;

/*
 * pixelformat: every pixel is 4bytes, in the following order: rgba,
 * red ist the least significant byte in every pixel
 * the first row is the uppermost row in the image
 */

BMPError bmp_dump(int fd, int w, int h, const uint32_t *pixels);

BMPError bmp_read(int fd, int *w, int *h, uint32_t **pixels);

#ifdef __cplusplus
}
#endif
    
#endif
