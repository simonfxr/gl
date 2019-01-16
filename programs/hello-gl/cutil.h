#ifdef __cplusplus
extern "C" {
#endif

#include <glad/glad.h>

void *
file_contents(const char *filename, GLint *length);
void *
read_tga(const char *filename, int *width, int *height);

#ifdef __cplusplus
}
#endif
