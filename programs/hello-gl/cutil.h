#ifdef __cplusplus
extern "C"
{
#endif

void *file_contents(const char *filename, GLint *length);
void *read_tga(const char *filename, int *width, int *height);

#ifdef __cplusplus
}
#endif
