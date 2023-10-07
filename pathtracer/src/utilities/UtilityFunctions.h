//
// Copyright (C) 2019-2023, Bayerische Motoren Werke Aktiengesellschaft (BMW AG)
//

/// @file      UtilityFunctions.h
/// @author    Bayerische Motoren Werke Aktiengesellschaft (BMW AG)
/// @copyright (C) 2019-2023, Bayerische Motoren Werke Aktiengesellschaft (BMW AG)
/// @date      2019-06-30
/// @brief     Plane vectors types and basic functions, usefull for different applications

#ifndef UTILITYFUNCTIONS_H
#define UTILITYFUNCTIONS_H

#include <algorithm>
#include <cmath>
#include <functional>
#include <tuple>
#include "memory.h"

#include "Constants.h"

/*
    NON-STL algorithms.
*/

/// @brief Aliases for primitive C-arrays.
using dvec4     = double[4];
using dvec3     = double[3];
using dvec2     = double[2];
using fvec4     = float[4];
using fvec3     = float[3];
using fvec2     = float[2];
using ivec4     = int[4];
using ivec3     = int[3];
using ivec2     = int[2];
using uivec4    = unsigned int[4];
using uivec3    = unsigned int[3];
using uivec2    = unsigned int[2];
using cvec4     = char[4];
using cvec3     = char[3];
using cvec2     = char[2];
using ubvec4    = unsigned char[4];
using ubvec3    = unsigned char[3];
using ubvec2    = unsigned char[2];
using dmat4     = double[4][4];
using dmat3     = double[3][3];
using dmat2     = double[2][2];
using fmat4     = float[4][4];
using fmat3     = float[3][3];
using fmat2     = float[2][2];

/// @brief Type for complex numbers.
using Complex   = std::complex<float>;

/// @brief Support for @c float based functions.
using Func1D    = float (*)(float);
using Func2D    = float (*)(float, float);
using Func3D    = float (*)(float, float, float);


/// @section Vector operations.

/// @brief Casts values from @a C-array @p a of type @p T1 to type @p T2 and
/// writes the result to @p b .
template<typename T1, typename T2>
void castVector(const T1 a[], T2 b[], const int n) noexcept
{
    for (int i = 0; i < n; ++i) b[i] = static_cast<T2>(a[i]);
}

/// @brief Assigns @a C-array @p dst with 2 first values from @p src .
template<typename T>
void assignVector2(T dst[], const T src[]) noexcept
{
    dst[0] = src[0];
    dst[1] = src[1];
}

/// @brief Assigns @a C-array @p dst with 3 first values from @p src .
template<typename T>
void assignVector3(T dst[], const T src[]) noexcept
{
    dst[0] = src[0];
    dst[1] = src[1];
    dst[2] = src[2];
}

/// @brief Assigns @a C-array @p dst with 4 first values from @p src .
template<typename T>
void assignVector4(T dst[], const T src[]) noexcept
{
    dst[0] = src[0];
    dst[1] = src[1];
    dst[2] = src[2];
    dst[3] = src[3];
}

/// @brief Assigns @a C-array @p dst with values @p v0 and @p v1 .
template<typename T>
void assignVector(T dst[], T v0, T v1) noexcept
{
    dst[0] = v0;
    dst[1] = v1;
}

/// @brief Assigns @a C-array @p dst with values @p v0 , @p v1 and @p v2 .
template<typename T>
void assignVector(T dst[], T v0, T v1, T v2) noexcept
{
    dst[0] = v0;
    dst[1] = v1;
    dst[2] = v2;
}

/// @brief Assigns @a C-array @p dst with values @p v0 , @p v1 , @p v2 and @p v3 .
template<typename T>
void assignVector(T dst[], T v0, T v1, T v2, T v3) noexcept
{
    dst[0] = v0;
    dst[1] = v1;
    dst[2] = v2;
    dst[3] = v3;
}

/// @brief Computes @a cross-product of two 3D vectors @p a and @p b and writes
/// the result to @p r .
template<typename T>
void cross3(const T a[], const T b[], T r[]) noexcept
{
    assignVector(r, a[1]*b[2] - a[2]*b[1], a[2]*b[0] - a[0]*b[2], a[0]*b[1] - a[1]*b[0]);
}

/// @brief Returns @a Z-component of the @a cross-product of two vectors @p a and @p b .
template<typename T>
constexpr T cross3Z(const T a[], const T b[]) noexcept
{
    return a[0]*b[1] - a[1]*b[0];
}

/// @brief Computes @a dot-product of two 2D vectors @p a and @p b .
template<typename T>
constexpr T dot2(const T a[], const T b[]) noexcept
{
    return a[0]*b[0] + a[1]*b[1];
}

/// @brief Computes @a dot-product of two 3D vectors @p a and @p b .
template<typename T>
constexpr T dot3(const T a[], const T b[]) noexcept
{
    return a[0]*b[0] + a[1]*b[1] + a[2]*b[2];
}

/// @brief Computes the physical length of 2D vector @p a .
template<typename T>
T length2(const T a[]) noexcept
{
    return std::sqrt(a[0]*a[0] + a[1]*a[1]);
}

/// @brief Computes the physical length of 23D vector @p a .
template<typename T>
T length3(const T a[]) noexcept
{
    return std::sqrt(a[0]*a[0] + a[1]*a[1] + a[2]*a[2]);
}

/// @brief Computes the physical distance between two 2D points @p a and @p b .
template<typename T>
T distance2(const T a[], const T b[]) noexcept
{
    T c[2] = {a[0]-b[0], a[1]-b[1]};
    return length2(c);
}

/// @brief Computes the physical distance between two 3D points @p a and @p b .
template<typename T>
T distance3(const T a[], const T b[]) noexcept
{
    T c[3] = {a[0]-b[0], a[1]-b[1], a[2]-b[2]};
    return length3(c);
}

/// @brief Computes the square of the physical distance between two 2D points @p a and @p b .
template<typename T>
T squareDistance2(const T a[], const T b[]) noexcept
{
    T c[2] = {a[0]-b[0], a[1]-b[1]};
    return dot2(c, c);
}

/// @brief Computes the square of the physical distance between two 3D points @p a and @p b .
template<typename T>
T squareDistance3(const T a[], const T b[]) noexcept
{
    T c[3] = {a[0]-b[0], a[1]-b[1], a[2]-b[2]};
    return dot3(c, c);
}

/// @brief Normalizes 3D vector @p a to the @b unit length and writes the result to @p r .
template<typename T>
void normalize3(const T a[], T r[]) noexcept
{
    T L = length3(a);
    if (L != static_cast<T>(0.0)) {
        L = static_cast<T>(1.0) / L;
        assignVector(r, a[0]*L, a[1]*L, a[2]*L);
    }
    else assignVector(r, static_cast<T>(1.0), static_cast<T>(0.0), static_cast<T>(0.0)); // Default unit vector
}

/// @brief Normalizes 3D vector @p a to the @b unit length.
template<typename T>
void normalize3(T a[]) noexcept
{
    normalize3(a, a);
}

/// @brief Normalizes 3D vector @p a to the @b unit length, if its current length deviates
/// from 1 by not more than @p eps and writes the result to @p r .
template<typename T>
void normalizeIfNeeded3(const T a[], T r[], T eps = static_cast<T>(10.0)*std::numeric_limits<T>::epsilon()) noexcept
{
    const auto one = static_cast<T>(1.0);
    T sL = dot3(a, a);

    if (sL < one + eps && sL > one - eps) {
        assignVector3(r, a);
        return;
    }

    T L = std::sqrt(sL);
    if (L != static_cast<T>(0.0)) {
        L = one / L;
        assignVector(r, a[0]*L, a[1]*L, a[2]*L);
    }
    else assignVector(r, one, static_cast<T>(0.0), static_cast<T>(0.0)); // Default unit vector
}

/// @brief Normalizes 3D vector @p a to the @b unit length, if its current length deviates
/// from 1 by not more than @p eps .
template<typename T>
void normalizeIfNeeded3(T a[], T eps = static_cast<T>(10.0)*std::numeric_limits<T>::epsilon()) noexcept
{
    normalizeIfNeeded3(a, a, eps);
}

/// @section Matrix operations.

/// @brief Assigns @a identity matrix of size @p N and type @p T to @p M .
template<typename T, int N>
void assignIdentityMatrix(T M[][N])
{
    std::memset(M, 0, sizeof(T)*N*N);
    for (int i = 0; i < N; M[i][i] = static_cast<T>(1.0), ++i);
}

/// @brief Computes a product of two 3D matrices @p A and @p B and writes
/// the result to @p C .
template<typename T>
void multMatrix3(const T A[][3], const T B[][3], T C[][3]) noexcept
{
    for (int i=0; i<3; ++i)
        for (int j=0; j<3; j++)
            C[i][j]=A[i][0]*B[0][j]+A[i][1]*B[1][j]+A[i][2]*B[2][j];
}

/// @brief Computes a product of two square matrices @p A and @p B of size @p N
/// and writes the result to @p C .
template<typename T, int N>
void multSquareMatrix(const T A[][N], const T B[][N], T C[][N]) noexcept
{
    int i,j,k;
    for (i=0; i<N; ++i)
        for (j=0; j<N; j++)
        {
            C[i][j]=static_cast<T>(0.0);
            for (k=0; k<N; ++k)
                C[i][j]+=A[i][k]*B[k][j];
        }
}

/// @brief Computes a product of 3D matrix @p A and 3D vector @p V and writes
/// the result to @p R .
template<typename T>
void multVector3(const T A[][3], const T V[], T R[]) noexcept
{
    for (int i=0; i<3; ++i) R[i]=A[i][0]*V[0]+A[i][1]*V[1]+A[i][2]*V[2];
}

/// @brief Computes a product of 3D vector @p V and 3D matrix @p A and writes
/// the result to @p R .
template<typename T>
void multVector3(const T V[], const T A[][3], T R[]) noexcept
{
    for (int i=0; i<3; ++i) R[i]=A[0][i]*V[0]+A[1][i]*V[1]+A[2][i]*V[2];
}

/// @brief Computes a product of 4D matrix @p A and 3D vector @p V and writes
/// the result to 3D vector @p R . Matrix @p A is treated as a @a transformation
/// matrix composed of rotation and scaling as the first 3 rows and columns and
/// a shift vector as a set of first 3 entries of the last column.
template<typename T>
void multVector3(const T A[][4], const T V[], T R[]) noexcept
{
    for (int i=0; i<3; ++i) R[i]=A[i][0]*V[0]+A[i][1]*V[1]+A[i][2]*V[2]+A[i][3];
}

/// @brief Computes a product of @a N-dimensional square matrix @p A and
/// @a N-dimensional vector @p V and writes the result to @p R .
template<typename T, int N>
void multVectorSquare(const T A[][N], const T V[], T R[]) noexcept
{
    for (int i=0; i<N; ++i)
    {
        R[i]=static_cast<T>(0.0);
        for (int j=0; j<N; ++j) R[i]+=A[i][j]*V[j];
    }
}

/// @brief Computes a product of @a N-dimensional vector @p V and
/// @a N-dimensional square matrix @p A and writes the result to @p R .
template<typename T, int N>
void multVectorSquare(const T V[], const T A[][N], T R[]) noexcept
{
    for (int i = 0; i < N; ++i) {
        R[i] = static_cast<T>(0.0);
        for (int j=0; j<N; ++j) R[i] += A[j][i]*V[j];
    }
}

/// @brief Computes the @a determinant of 3D matrix @p M .
template<typename T>
constexpr T detMatrix3(const T M[][3]) noexcept
{
    return M[0][0]*M[1][1]*M[2][2] + M[0][1]*M[1][2]*M[2][0] + M[0][2]*M[1][0]*M[2][1] -
           M[0][2]*M[1][1]*M[2][0] - M[0][1]*M[1][0]*M[2][2] - M[0][0]*M[1][2]*M[2][1];
}

/// @brief Computes @a inverse matrix for the given 3D matrix @p M and writes
/// the result to @p R .
template<typename T>
void inverseMatrix3(const T M[][3], T R[][3]) noexcept
{
    T k = 1.0 / detMatrix3(M);
    R[0][0] = k*( M[1][1]*M[2][2]-M[1][2]*M[2][1]);	R[0][1] = k*(-M[0][1]*M[2][2]+M[0][2]*M[2][1]);	R[0][2] = k*( M[0][1]*M[1][2]-M[0][2]*M[1][1]);
    R[1][0] = k*(-M[1][0]*M[2][2]+M[1][2]*M[2][0]);	R[1][1] = k*( M[0][0]*M[2][2]-M[0][2]*M[2][0]);	R[1][2] = k*(-M[0][0]*M[1][2]+M[0][2]*M[1][0]);
    R[2][0] = k*( M[1][0]*M[2][1]-M[1][1]*M[2][0]);	R[2][1] = k*(-M[0][0]*M[2][1]+M[0][1]*M[2][0]);	R[2][2] = k*( M[0][0]*M[1][1]-M[0][1]*M[1][0]);
}

/// @brief Computes @a inverse matrix for the given @a N-dimensional matrix @p M
/// and writes the result to @p R .
template<typename T, int N>
void transposeMatrix(const T M[][N], T R[][N]) noexcept
{
    for (int i = 0; i < N; ++i)
        for (int j = 0; j < N; ++j)
            R[i][j] = M[j][i];
}

/// @brief Computes the @a rotation matrix @p M out of the given quaternion @p q . 
template<class T>
void matrixFromQuaternion(const T q[], T M[][3]) noexcept
{
    constexpr T one = static_cast<T>(1.0);
    constexpr T two = static_cast<T>(2.0);

    M[0][0] = one - two*(q[1]*q[1] + q[2]*q[2]);
    M[0][1] = two*(q[0]*q[1] - q[2]*q[3]);
    M[0][2] = two*(q[0]*q[2] + q[1]*q[3]);
    M[1][0] = two*(q[0]*q[1] + q[2]*q[3]);
    M[1][1] = one - two*(q[0]*q[0] + q[2]*q[2]);
    M[1][2] = two*(q[1]*q[2] - q[0]*q[3]);
    M[2][0] = two*(q[0]*q[2] - q[1]*q[3]);
    M[2][1] = two*(q[1]*q[2] + q[0]*q[3]);
    M[2][2] = one - two*(q[0]*q[0] + q[1]*q[1]);
}


/// @section Range utilities.

/// @brief Does nothing.
template<typename T>
void deleteContainerElement(T &) noexcept {}

/// @brief Deletes an object under pointer @p element and assigns it
/// to @c nullptr .
template<typename T>
void deleteContainerElement(T *&element) noexcept
{
    delete element;
    element = nullptr;
}

/// @brief Deletes an object stored as the second element in a @a pair @p element .
template<typename K, typename V>
void deleteContainerElement(std::pair<K, V> &element) noexcept
{
    deleteContainerElement(element.second);
}

/// @brief Calls destructors for all pointers stored in the @a STL container @p container
/// and deletes all pointers in this container.
template<typename T>
void deleteContainerWithPointers(T &container) noexcept
{
    for (auto& v : container) deleteContainerElement(v);
    T().swap(container);
}

/// @brief Utility functor which scales any given value of type @c T by
/// some constant @p value of the same type.
template<typename T>
struct Scale final
{
    /// @brief Constructs new instance of @c Scale out of value @p v .
    constexpr explicit Scale(T v = 1.0f) noexcept : value(v) {}
    /// @brief Returns a product of @p entry and underlying @ref value .
    constexpr T operator()(T entry) const noexcept { return value * entry; }
    /// @brief Scaling value.
    T value;
};

using Scaled    = Scale<double>;
using Scalef    = Scale<float>;
using Scalei    = Scale<int>;


/// @brief @a Minimum function.
template <class T>
constexpr T min(T a, T b) noexcept
{
    return a < b ? a : b;
}

/// @brief @a Maximum function.
template <class T>
constexpr T max(T a, T b) noexcept
{
    return a > b ? a : b;
}

/// @brief Clamps the value @p value between two other ones @p lower and @p upper ,
/// assuming that @p lower is less or equal to @p upper .
template <class T>
constexpr T clamp(T value, T lower, T upper) noexcept
{
    return max(lower, min(value, upper));
}

/// @brief Clamps the @c float value @p value between 0 and 1.
constexpr float clamp01f(float value) noexcept
{
    return clamp(value, 0.0f, 1.0f);
}

/// @brief Returns the result of mathematical expression @p k * @p x + @p c .
template<typename T>
constexpr T linearFunction(T k, T c, T x) noexcept // Computes k*x + c
{
    return k * x + c;
}

/// @brief Linearly interpolates between @p lower and @p upper with the weight @p value .
template <class T>
constexpr T lerp(T lower, T upper, T value) noexcept
{
    return linearFunction(upper - lower, lower, value);
}

/// @brief Reflects 3D vector @p vI w.r.t. @p vN and writes the result to @p vO .
template<typename T>
void reflect3(const T *vI, const T *vN, T *vO) noexcept
{
    const T w = static_cast<T>(2.0) * dot3(vI, vN);
    for (int i = 0; i < 3; ++i) vO[i] = vI[i] - w*vN[i];
}

/// @brief Refracts 3D vector @p vI w.r.t. @p vN according to a given @a index-of-refraction
/// @p eta and writes the result to @p vR .
template<typename T>
bool refract3(const T *vI, const T *vN, float eta, T *vR) noexcept
{
    T N_I = dot3(vN, vI);
    T k = 1.0f - eta*eta*(1.0f - N_I*N_I);
    if (k < 0.0f) {
        assignVector(vR, -vI[0], -vI[1], -vI[2]);
        return false; // refracted ray does not exist
    }
    else {
        k = eta*N_I + std::sqrt(k);
        for (int i = 0; i < 3; ++i) vR[i] = eta*vI[i] - k*vN[i];
        return true;
    }
}

/// @brief Creates a 4D transformation matrix out of given @a quaternion @p Q ,
/// 3D @a translation vector @p T and 3D @a scaling vector @p S .
template <class C>
void transformationFromQTS(C M[][4], const C *Q=nullptr, const C *T=nullptr, const C *S=nullptr)
{
    constexpr C zer = static_cast<C>(0.0);
    constexpr C one = static_cast<C>(1.0);

    C R[3][3];
    (Q) ? matrixFromQuaternion(Q, R) : assignIdentityMatrix(R);
    C Tv[3] = {zer,zer,zer};
    if (T) assignVector3(Tv, T);
    C Sv[3] = {one,one,one};
    if (S) assignVector3(Sv, S);

    assignVector(M[0], R[0][0]*Sv[0], R[0][1]*Sv[1], R[0][2]*Sv[2], Tv[0]);
    assignVector(M[1], R[1][0]*Sv[0], R[1][1]*Sv[1], R[1][2]*Sv[2], Tv[1]);
    assignVector(M[2], R[2][0]*Sv[0], R[2][1]*Sv[1], R[2][2]*Sv[2], Tv[2]);
    assignVector(M[3], zer,           zer,           zer,           one);
}

/// @brief Inverts 4D transformation matrix @p M and writes the result to @p Minv .
template <class T>
void transformationInverse(const T M[][4], T Minv[][4])
{
    constexpr T zer = static_cast<T>(0.0);
    constexpr T one = static_cast<T>(1.0);

    T RS[3][3] = {{M[0][0], M[0][1], M[0][2]},
                  {M[1][0], M[1][1], M[1][2]},
                  {M[2][0], M[2][1], M[2][2]}};
    T RSi[3][3];
    inverseMatrix3(RS, RSi);
    T TV[3] = {M[0][3], M[1][3], M[2][3]}, TVi[3];
    multVector3(RSi, TV, TVi);

    assignVector(Minv[0], RSi[0][0], RSi[0][1], RSi[0][2], -TVi[0]);
    assignVector(Minv[1], RSi[1][0], RSi[1][1], RSi[1][2], -TVi[1]);
    assignVector(Minv[2], RSi[2][0], RSi[2][1], RSi[2][2], -TVi[2]);
    assignVector(Minv[3], zer,       zer,       zer,       one);
}

/// @brief Computes flat normal of a triangle represented by its 3 vertices
/// @p V0 , @p V1 and @p V2 and writes the result to @p N .
template<typename T>
void triangleNormal(const T *V0, const T *V1, const T *V2, T *N) noexcept
{
    const T E1[3] = {V1[0] - V0[0], V1[1] - V0[1], V1[2] - V0[2]};
    const T E2[3] = {V2[0] - V0[0], V2[1] - V0[1], V2[2] - V0[2]};
    cross3(E1, E2, N);
}

/// @brief Computes the area of a triangle represented by its 3 vertices
/// @p V0 , @p V1 and @p V2 .
template<typename T>
T triangleArea(const T *V0, const T *V1, const T *V2) noexcept
{
    T N[3];
    triangleNormal(V0, V1, V2, N);
    return length3(N) / static_cast<T>(2.0);
}

#endif // !UTILITYFUNCTIONS_H
