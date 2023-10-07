//
// Copyright (C) 2019-2023, Bayerische Motoren Werke Aktiengesellschaft (BMW AG)
//

/// @file      DistanceRandom.h
/// @author    Bayerische Motoren Werke Aktiengesellschaft (BMW AG)
/// @copyright (C) 2019-2023, Bayerische Motoren Werke Aktiengesellschaft (BMW AG)
/// @date      2022-04-22
/// @brief     Random module for light support

#ifndef DISTANCERANDOM_H
#define DISTANCERANDOM_H

#include <cstdlib>
#include <numeric>

/**
 * @brief @c DistanceRandom class stores a set of distances (weights, squares, etc.)
 * assotiated with object indices. Usign this class, random objects could be picked
 * based on their distances (weights, squares, etc.).
 */ 
template <typename T>
class DistanceRandom final
{
    /// @brief Number of objects to store.
    int m_iN;
    /// @brief Sum of distances (weights, squares, etc.) of all objects inside the class.
    T m_maxD;
    /// @brief Array of length @ref DistanceRandom::m_iN of distances (weights, squares, etc.)
    /// associated with every object and @a CDF after calculation ( @a distance-data ).
    T *m_distances;

public:
    /// @publicsection Basic interface (e.g. @a -ctors).

    /// @brief Creates new empty instance of @c DistanceRandom .
    DistanceRandom();
    /// @brief -Dtor.
    ~DistanceRandom();

    /// @brief Computes @a CDF based on @a distance-data and rewrites it with distances @a CDF.
    void calculate();

public:
    /// @publicsection Getters and setters.

    /// @brief Sets a total number of objects.
    void setCount(int N);
    /// @brief Returns the total number of objects.
    int getCount() const noexcept { return m_iN; }

    /// @brief Sets a distance (weight, square, etc.) @p d to the object, associated with index @p id .
    void setDistance(int id, T d) { m_distances[id] = d; }
    /// @brief Returns the distance (weight, square, etc.), associated with index @p id .
    T getDistance(int id) const { return m_distances[id]; }

    /// @brief Returns an index of the object, based on random clamped value @p rndVal in [0, 1).
    int getRandom(T rndVal) const noexcept; // rndVal - uniform in [0;1)
    /// @brief Returns an index of the object, based on random distance @p d in [0, @ref getMaxD() ).
    int getValue(T d) const noexcept; // d: 0..m_maxD;
    /// @brief Returns the probability of the index sample to be returned in case of uniform distribution
    /// of @p rndVal in getRandom and d in getValue
    T getPdf(int sample) const noexcept;
    /// @brief Returns the sum of distances (weights, squares, etc.) of all objects.
    T getMaxD() const noexcept { return m_maxD; }
};


template <typename T>
inline DistanceRandom<T>::DistanceRandom() :
    m_iN(0), m_maxD(static_cast<T>(0)), m_distances(nullptr)
{}

template <typename T>
inline void DistanceRandom<T>::setCount(int N)
{
    m_iN = N;
    delete[] m_distances;
    m_distances = nullptr;
    if (N <= 0) return;
    m_distances = new T[m_iN];
}

template <typename T>
inline void DistanceRandom<T>::calculate()
{
    std::partial_sum(m_distances, m_distances + m_iN, m_distances);
    m_maxD = m_distances[m_iN - 1];
}

template <typename T>
inline int DistanceRandom<T>::getRandom(T rndVal) const noexcept
{
    return getValue(rndVal*m_maxD);
}

template <typename T>
inline int DistanceRandom<T>::getValue(T d) const noexcept
{    
    if (m_iN<=0) return -1;
    if (m_iN==1) return 0;    
    if (d<=m_distances[0]) return 0;
    if (d>=m_distances[m_iN-2]) return m_iN-1;
    int L=0;
    int R=m_iN-1;
    int C;
    while (L<R)
    {
        C=(L+R)/2;
        if (d<=m_distances[C]) R=C;
        else L=C+1;
    }
    return (L+R)/2;
}

template <typename T>
inline T DistanceRandom<T>::getPdf(int sample) const noexcept
{
    if (sample<0 || sample>=m_iN) return static_cast<T>(0.0);
    return (m_distances[sample]-(sample>0?m_distances[sample-1]:static_cast<T>(0.0)))/m_maxD;
}

template <typename T>
inline DistanceRandom<T>::~DistanceRandom()
{
    delete[] m_distances;
}

#endif // !DISTANCERANDOM_H
