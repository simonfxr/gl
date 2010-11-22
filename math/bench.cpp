#include <iostream>
#include <string>
#include <sstream>
#include <ctime>
#include <climits>

#include "mat4.hpp"

namespace {

    mat4 mult(uint32 size, mat4 *mats) {
        mat4 A = identity;
        for (uint32 i = 0; i < size; ++i)
            A = A * mats[i];
        return A;
    }
}

using namespace std;

int main(int argc, char *argv[]) {

    stringstream arg1(argv[1] != 0 ? argv[1] : "");
    uint32 size = 1000;
    arg1 >> size;

    mat4 *mats = new mat4[size];

    for (uint32 i = 0; i < size; ++i)
        mats[i] = identity;

    cout << "multiplying " << size << " matrices" << endl;

    clock_t t1 = clock();
    mat4 M = mult(size, mats);
    clock_t t2 = clock();

    delete mats;

    cout << "done in " << (double) (t2 - t1) / CLOCKS_PER_SEC << " seconds" << endl;
   
    cout << vec4(M.c1).x << endl;

    return 0;
}
