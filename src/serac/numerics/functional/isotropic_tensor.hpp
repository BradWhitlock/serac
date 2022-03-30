// Copyright (c) 2019-2022, Lawrence Livermore National Security, LLC and
// other Serac Project Developers. See the top-level LICENSE file for
// details.
//
// SPDX-License-Identifier: (BSD-3-Clause)

/**
 * @file isotropic_tensor.hpp
 *
 * @brief Implementation of isotropic tensor classes
 * 
 * @note Do not include this file directly, it is included by tensor.hpp
 */

#pragma once

#include <iostream>
#include <type_traits>

namespace serac {

/**
 * @brief an object representing a highly symmetric kind of tensor, 
 *        that is interoperable with `serac::tensor`, but uses less memory
 *        and performs less calculation than its dense counterpart
 * @tparam T The type stored at each index
 * @tparam n The dimensions of the tensor
 */
template <typename T, int... n>
struct isotropic_tensor;

///
template <typename T, int n>
struct isotropic_tensor<T, n> {
  static_assert(::detail::always_false<T>{}, "error: there is no such thing as a rank-1 isotropic tensor!");
};

/**
 * @brief a rank-2 isotropic tensor is essentially just the Identity matrix, with a constant of proportionality
 * @tparam T The type stored at each index
 * @tparam n The dimensions of the tensor
 */
template <typename T, int m>
struct isotropic_tensor<T, m, m> {
  SERAC_HOST_DEVICE constexpr T operator()(int i, int j) const { return (i == j) * value; }
  T                             value;
};

/**
 * @brief return the identity matrix of the specified size
 * @tparam m the number of rows and columns
 */
template <int m>
SERAC_HOST_DEVICE constexpr isotropic_tensor<double, m, m> Identity()
{
  return isotropic_tensor<double, m, m>{1.0};
}

/**
 * @brief scalar multiplication
 * 
 * @tparam S the type of the left operand, `scale`
 * @tparam T the type of the isotropic tensor
 * @tparam m the number of rows and columns in the isotropic tensor
 * @param scale the value that multiplies each entry of I (from the left)
 * @param I the isotropic tensor being scaled
 * @return a new isotropic tensor equal to the product of scale * I
 */
template <typename S, typename T, int m>
SERAC_HOST_DEVICE constexpr auto operator*(S scale, isotropic_tensor<T, m, m> I)
{
  return isotropic_tensor<decltype(S{} * T{}), m, m>{I.value * scale};
}

/// @overload
template <typename S, typename T, int m>
SERAC_HOST_DEVICE constexpr auto operator*(isotropic_tensor<T, m, m> I, S scale)
{
  return isotropic_tensor<decltype(S{}, T{}), m, m>{I.value * scale};
}

/**
 * @brief addition of isotropic tensors
 * 
 * @tparam S the type of the left isotropic tensor
 * @tparam T the type of the right isotropic tensor
 * @tparam m the number of rows and columns in each isotropic tensor 
 * @param I1 the left operand
 * @param I2 the right operand
 * @return a new isotropic tensor equal to the sum of I1 and I2
 */
template <typename S, typename T, int m>
SERAC_HOST_DEVICE constexpr auto operator+(isotropic_tensor<S, m, m> I1, isotropic_tensor<T, m, m> I2)
{
  return isotropic_tensor<decltype(S{} + T{}), m, m>{I1.value + I2.value};
}

/**
 * @brief difference of isotropic tensors
 * 
 * @tparam S the type of the left isotropic tensor
 * @tparam T the type of the right isotropic tensor
 * @tparam m the number of rows and columns in each isotropic tensor 
 * @param I1 the left operand
 * @param I2 the right operand
 * @return a new isotropic tensor equal to the difference of I1 and I2
 */
template <typename S, typename T, int m>
SERAC_HOST_DEVICE constexpr auto operator-(isotropic_tensor<S, m, m> I1, isotropic_tensor<T, m, m> I2)
{
  return isotropic_tensor<decltype(S{} - T{}), m, m>{I1.value - I2.value};
}

/**
 * @brief sum of isotropic and (nonisotropic) tensor
 * 
 * @tparam S the type of the left isotropic tensor
 * @tparam T the type of the right tensor
 * @tparam m the number of rows and columns in each tensor 
 * @param I1 the left operand
 * @param A the (full) right operand
 * @return a new tensor equal to the sum of I and A
 */
template <typename S, typename T, int m>
SERAC_HOST_DEVICE constexpr auto operator+(const isotropic_tensor<S, m, m>& I, const tensor<T, m, m>& A)
{
  tensor<decltype(S{} + T{}), m, m> output{};
  for (int i = 0; i < m; i++) {
    for (int j = 0; j < m; j++) {
      output[i][j] = I.value * (i == j) + A[i][j];
    }
  }
  return output;
}

/// @overload
template <typename S, typename T, int m>
SERAC_HOST_DEVICE constexpr auto operator+(const tensor<S, m, m>& A, const isotropic_tensor<T, m, m>& I)
{
  tensor<decltype(S{} + T{}), m, m> output{};
  for (int i = 0; i < m; i++) {
    for (int j = 0; j < m; j++) {
      output[i][j] = A[i][j] + I.value * (i == j);
    }
  }
  return output;
}

/**
 * @brief difference of isotropic and (nonisotropic) tensor
 * 
 * @tparam S the type of the left isotropic tensor
 * @tparam T the type of the right tensor
 * @tparam m the number of rows and columns in each tensor 
 * @param I1 the left operand
 * @param A the (full) right operand
 * @return a new tensor equal to the difference of I and A
 */
template <typename S, typename T, int m>
SERAC_HOST_DEVICE constexpr auto operator-(const isotropic_tensor<S, m, m>& I, const tensor<T, m, m>& A)
{
  tensor<decltype(S{} - T{}), m, m> output{};
  for (int i = 0; i < m; i++) {
    for (int j = 0; j < m; j++) {
      output[i][j] = I.value * (i == j) - A[i][j];
    }
  }
  return output;
}

/// @overload
template <typename S, typename T, int m>
SERAC_HOST_DEVICE constexpr auto operator-(const tensor<S, m, m>& A, const isotropic_tensor<T, m, m>& I)
{
  tensor<decltype(S{} - T{}), m, m> output{};
  for (int i = 0; i < m; i++) {
    for (int j = 0; j < m; j++) {
      output[i][j] = A[i][j] - I.value * (i == j);
    }
  }
  return output;
}

/**
 * @brief dot product between an isotropic and (nonisotropic) tensor
 * 
 * @tparam S the types stored in isotropic tensor
 * @tparam T the types stored in tensor
 * @tparam m the number of rows and columns in I
 * @tparam n the trailing dimensions of A
 * @param I the left operand
 * @param A the (full) right operand
 * @return a new tensor equal to the index notation expression: output(i,...) := I(i,j) * A(j,...)
 */
template <typename S, typename T, int m, int... n>
SERAC_HOST_DEVICE constexpr auto dot(const isotropic_tensor<S, m, m>& I, const tensor<T, m, n...>& A)
{
  return I.value * A;
}

/// @overload
template <typename S, typename T, int m, int... n>
SERAC_HOST_DEVICE constexpr auto dot(const tensor<S, n...>& A, isotropic_tensor<T, m, m> I)
{
  constexpr int dimensions[sizeof...(n)] = {n...};
  static_assert(dimensions[sizeof...(n) - 1] == m);
  return A * I.value;
}

/**
 * @brief double-dot product between an isotropic and (nonisotropic) tensor
 * 
 * @tparam S the types stored in isotropic tensor
 * @tparam T the types stored in tensor
 * @tparam m the number of rows and columns in I, A
 * @param I the left operand
 * @param A the (full) right operand
 * @return a new tensor equal to the index notation expression: 
 *    output := I(i,j) * A(i,j) \propto tr(A)
 */
template <typename S, typename T, int m>
SERAC_HOST_DEVICE constexpr auto ddot(const isotropic_tensor<S, m, m>& I, const tensor<T, m, m>& A)
{
  return I.value * tr(A);
}

/**
 * @brief return the symmetric part of an isotropic tensor
 * 
 * @tparam T the types stored in the isotropic tensor
 * @tparam m the number of rows and columns in I
 * @param I the isotropic tensor to symmetrize
 * @return a copy of I (isotropic matrices are already symmetric)
 */
template <typename T, int m>
SERAC_HOST_DEVICE constexpr auto sym(const isotropic_tensor<T, m, m>& I)
{
  return I;
}

/**
 * @brief return the antisymmetric part of an isotropic tensor
 * 
 * @tparam T the types stored in the isotropic tensor
 * @tparam m the number of rows and columns in I
 * @param I the isotropic tensor to anti-symmetrize
 * @return `zero` (isotropic matrices are symmetric, so the antisymmetric part is identically zero)
 */
template <typename T, int m>
SERAC_HOST_DEVICE constexpr auto antisym(const isotropic_tensor<T, m, m>&)
{
  return zero{};
}

/**
 * @brief calculate the trace of an isotropic tensor
 * 
 * @tparam T the types stored in the isotropic tensor
 * @tparam m the number of rows and columns in I
 * @param I the isotropic tensor to compute the trace of
 * @return the sum of the diagonal entries of I
 */
template <typename T, int m>
SERAC_HOST_DEVICE constexpr auto tr(const isotropic_tensor<T, m, m>& I)
{
  return I.value * m;
}

/**
 * @brief return the transpose of an isotropic tensor
 * 
 * @tparam T the types stored in the isotropic tensor
 * @tparam m the number of rows and columns in I
 * @param I the isotropic tensor to compute the trace of
 * @return a copy of I (isotropic matrices are symmetric)
 */
template <typename T, int m>
SERAC_HOST_DEVICE constexpr auto transpose(const isotropic_tensor<T, m, m>& I)
{
  return I;
}

/**
 * @brief compute the determinant of an isotropic tensor
 * 
 * @tparam T the types stored in the isotropic tensor
 * @tparam m the number of rows and columns in I
 * @param I the isotropic tensor to compute the determinant of
 */
template <typename T, int m>
SERAC_HOST_DEVICE constexpr auto det(const isotropic_tensor<T, m, m>& I)
{
  return std::pow(I.value, m);
}

/**
 * @brief compute the Frobenius norm (sqrt(tr(dot(transpose(I), I)))) of an isotropic tensor
 * 
 * @tparam T the types stored in the isotropic tensor
 * @tparam m the number of rows and columns in I
 * @param I the isotropic tensor to compute the norm of
 */
template <typename T, int m>
SERAC_HOST_DEVICE constexpr auto norm(const isotropic_tensor<T, m, m>& I)
{
  return sqrt(I.value * I.value * m);
}

/**
 * @brief compute the squared Frobenius norm (tr(dot(transpose(I), I))) of an isotropic tensor
 * 
 * @tparam T the types stored in the isotropic tensor
 * @tparam m the number of rows and columns in I
 * @param I the isotropic tensor to compute the squared norm of
 */
template <typename T, int m>
SERAC_HOST_DEVICE constexpr auto sqnorm(const isotropic_tensor<T, m, m>& I)
{
  return I.value * I.value * m;
}

// rank-3 isotropic tensors are just the alternating symbol
template <typename T>
struct isotropic_tensor<T, 3, 3, 3> {
  SERAC_HOST_DEVICE constexpr T operator()(int i, int j, int k) const
  {
    return 0.5 * (i - j) * (j - k) * (k - i) * value;
  }
  T value;
};

// there are 3 linearly-independent rank-4 isotropic tensors,
// so the general one will be some linear combination of them
template <typename T, int m>
struct isotropic_tensor<T, m, m, m, m> {
  T c1, c2, c3;

  SERAC_HOST_DEVICE constexpr T operator()(int i, int j, int k, int l) const
  {
    return c1 * (i == j) * (k == l) + c2 * ((i == k) * (j == l) + (i == l) * (j == k)) * 0.5 +
           c3 * ((i == k) * (j == l) - (i == l) * (j == k)) * 0.5;
  }
};

template <int m>
SERAC_HOST_DEVICE constexpr auto SymmetricIdentity()
{
  return isotropic_tensor<double, m, m, m, m>{0.0, 1.0, 0.0};
}

template <int m>
SERAC_HOST_DEVICE constexpr auto AntisymmetricIdentity()
{
  return isotropic_tensor<double, m, m, m, m>{0.0, 0.0, 1.0};
}

template <typename S, typename T, int m>
SERAC_HOST_DEVICE constexpr auto operator*(S scale, isotropic_tensor<T, m, m, m, m> I)
{
  return isotropic_tensor<decltype(S{} * T{}), m, m, m, m>{I.c1 * scale, I.c2 * scale, I.c3 * scale};
}

template <typename S, typename T, int m>
SERAC_HOST_DEVICE constexpr auto operator*(isotropic_tensor<S, m, m, m, m> I, T scale)
{
  return isotropic_tensor<decltype(S{} * T{}), m, m, m, m>{I.c1 * scale, I.c2 * scale, I.c3 * scale};
}

template <typename S, typename T, int m>
SERAC_HOST_DEVICE constexpr auto operator+(isotropic_tensor<S, m, m, m, m> I1, isotropic_tensor<T, m, m, m, m> I2)
{
  return isotropic_tensor<decltype(S{} + T{}), m, m, m, m>{I1.c1 + I2.c1, I1.c2 + I2.c2, I1.c3 + I2.c3};
}

template <typename S, typename T, int m>
SERAC_HOST_DEVICE constexpr auto operator-(isotropic_tensor<S, m, m, m, m> I1, isotropic_tensor<T, m, m, m, m> I2)
{
  return isotropic_tensor<decltype(S{} - T{}), m, m, m, m>{I1.c1 - I2.c1, I1.c2 - I2.c2, I1.c3 - I2.c3};
}

template <typename S, typename T, int m, int... n>
SERAC_HOST_DEVICE constexpr auto ddot(const isotropic_tensor<S, m, m, m, m>& I, const tensor<T, m, m, n...>& A)
{
  return I.c1 * tr(A) * Identity<m>() + I.c2 * sym(A) + I.c3 * antisym(A);
}

}  // namespace serac
