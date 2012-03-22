#include <windows.h>

extern int main(int, char **);

int WINAPI WinMain(HINSTANCE, HINSTANCE, LPSTR, INT) {
    return main(__argc, __argv);
}
