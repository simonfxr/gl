#ifndef _MATRIX_H
#define _MATRIX_H

#include "Vec.hpp"

template <typename Math, typename Matr, typename V, unsigned N, typename T>
struct Mat {
    T mat[N * N]; // column major

    Mat() {}

    struct RM {
        T data[N * N];
    };

    struct CM {
        T data[N * N];
    };

    Mat(RM A) {
        for (unsigned i = 0; i < M; ++i)
            for (unsigned j = 0; j < M; ++j)
                mat[i * M + j] = A.data[j * M + i];
    }

    Mat(CM A) {
        for (unsigned i = 0; i < N * N; ++i)
            mat[i] = A.data[i];
    }

    Matr mult(const Matr &B) const {
        const Matr &A = *this;
        Matr C;
        
        for (unsigned i = 0; i < N; ++i) {
            for (unsigned j = 0; j < N; ++j) {
                C(i, j) = 0;
                for (unsigned k = 0; k < N; ++k)
                    C(i, j) += A(i, k) * B(k, j);
            }
        }
                         
        return C;
    }

    V mult(const V &B) const {
        const Matr& A = *this;
        V C;

        for (unsigned i = 0; i < N; ++i) {
            C(i) = 0;
            for (unsigned k = 0; k < N; ++k)
                C(i) += A(i, k) * B(k);
        }
    }

    T operator()(int r, int c) const {
        return mat[M * c + r];
    }

    T& operator()(int r, int c) {
        return mat[M * c + r];
    }

    template<typename U>
    Matr operator *(const U& B) const {
        return mult(B);
    }

    template<typename U>
    Matr& operator *=(const U& B) {
        *this = mult(B);
        return *this;
    }

    static Matr identity() {
        Matr A;

        for (unsigned i = 0; i < M; ++i)
            for (unsigned j = 0; j < M; ++j)
                A(i, j) = i == j ? 1 : 0;

        return A;
    }
};


#endif
