// Copyright (C) 2009-2016 ETH Zurich
// Copyright (C) 2007?-2016 Center for Nanophase Materials Sciences, ORNL
// All rights reserved.
//
// See LICENSE.txt for terms of usage.
// See CITATION.txt for citation guidelines if you use this code for scientific publications.
//
// Author: Raffaele Solca' (rasolca@itp.phys.ethz.ch)
//
// This file tests the matrix operations with the Matrix<CPU> class.

#include "dca/linalg/matrixop.hpp"
#include <complex>
#include <stdexcept>
#include <string>
#include <utility>
#include "gtest/gtest.h"
#include "dca/linalg/matrix.hpp"
#include "dca/linalg/blas/blas3.hpp"
#include "cpu_test_util.hpp"

namespace testing {
template <typename ScalarType>
ScalarType conjugate(ScalarType x) {
  return x;
}

template <typename ScalarType>
std::complex<ScalarType> conjugate(std::complex<ScalarType> x) {
  return std::conj(x);
}

// Reference implementation of the matrix-matrix multiplication
// In/Out: c ('In' only if beta != 0)
// Preconditions: transa and transb should be one of the following: 'N', 'T', 'C',
//                a.nrRows() == c.nrRows() if transa == 'N', a.nrCols() == c.nrRows() otherwise,
//                b.nrCols() == c.nrCols() if transb == 'N', b.nrRows() == c.nrCols() otherwise,
//                ka == kb, where ka = a.nrCols() if transa == 'N', ka = a.nrRows() otherwise and
//                          kb = b.nrRows() if transb == 'N', kb = b.nrCols() otherwise.
// Remark: this implementation is inefficient.
//         It should only be used for testing purpose with small matrices.
template <typename ScalarTypeA, typename ScalarTypeB, typename ScalarTypeC>
void refGemm(char transa, char transb, ScalarTypeC alpha,
             const dca::linalg::Matrix<ScalarTypeA, dca::linalg::CPU>& a,
             const dca::linalg::Matrix<ScalarTypeB, dca::linalg::CPU>& b, ScalarTypeC beta,
             dca::linalg::Matrix<ScalarTypeC, dca::linalg::CPU>& c) {
  static_assert(std::is_same<ScalarTypeA, ScalarTypeC>::value ||
                    std::is_same<std::complex<ScalarTypeA>, ScalarTypeC>::value,
                "Wrong ScalarType for a");
  static_assert(std::is_same<ScalarTypeB, ScalarTypeC>::value ||
                    std::is_same<std::complex<ScalarTypeB>, ScalarTypeC>::value,
                "Wrong ScalarType for b");
  // Set the values for transa and transb equal 'N'.
  int ma = a.nrRows();
  int ka = a.nrCols();
  int kb = b.nrRows();
  int nb = b.nrCols();
  auto op_a = [transa, &a](int i, int j) {
    switch (transa) {
      case 'T':
        return a(j, i);
      case 'C':
        return testing::conjugate(a(j, i));
      default:
        return a(i, j);
    }
  };
  auto op_b = [transb, &b](int i, int j) {
    switch (transb) {
      case 'T':
        return b(j, i);
      case 'C':
        return testing::conjugate(b(j, i));
      default:
        return b(i, j);
    }
  };

  if (transa == 'T' || transa == 'C') {
    ma = a.nrCols();
    ka = a.nrRows();
  }
  else if (transa != 'N') {
    // transa is not 'N', 'T' or 'C'
    throw std::logic_error("Wrong value for transa");
  }
  if (transb == 'T' || transb == 'C') {
    kb = b.nrCols();
    nb = b.nrRows();
  }
  else if (transb != 'N') {
    // transb is not 'N', 'T' or 'C'
    throw std::logic_error("Wrong value for transb");
  }

  if (ma != c.nrRows() || ka != kb || nb != c.nrCols())
    throw std::logic_error("Wrong matrix sizes");

  for (int j = 0; j < c.nrCols(); ++j)
    for (int i = 0; i < c.nrRows(); ++i) {
      c(i, j) *= beta;
      for (int k = 0; k < ka; ++k)
        c(i, j) += alpha * op_a(i, k) * op_b(k, j);
    }
}
}  // testing

template <typename ScalarType>
class MatrixopRealCPUTest : public ::testing::Test {
public:
  static const ScalarType epsilon;
};
template <typename ScalarType>
const ScalarType MatrixopRealCPUTest<ScalarType>::epsilon = std::numeric_limits<ScalarType>::epsilon();

typedef ::testing::Types<float, double> FloatingPointTypes;
TYPED_TEST_CASE(MatrixopRealCPUTest, FloatingPointTypes);

TYPED_TEST(MatrixopRealCPUTest, Gemm) {
  using ScalarType = TypeParam;
  auto val_a = [](int i, int j) { return 3 * i - 2 * j; };
  auto val_b = [](int i, int j) { return 4 * i - 3 * j; };
  auto val_c = [](int i, int j) { return 2 * i - j; };
  {
    std::pair<int, int> size_a(2, 3);
    std::pair<int, int> size_b(3, 4);
    std::pair<int, int> size_c(2, 4);

    {
      dca::linalg::Matrix<ScalarType, dca::linalg::CPU> a(size_a);
      dca::linalg::Matrix<ScalarType, dca::linalg::CPU> b(size_b);
      dca::linalg::Matrix<ScalarType, dca::linalg::CPU> c(size_c);

      testing::setMatrixElements(a, val_a);
      testing::setMatrixElements(b, val_b);
      testing::setMatrixElements(c, val_c);

      dca::linalg::Matrix<ScalarType, dca::linalg::CPU> cref(c);

      testing::refGemm('N', 'N', ScalarType(1), a, b, ScalarType(0), cref);
      dca::linalg::matrixop::gemm(a, b, c);

      for (int j = 0; j < c.nrCols(); ++j)
        for (int i = 0; i < c.nrRows(); ++i) {
          EXPECT_NEAR(cref(i, j), c(i, j), 500 * this->epsilon);
        }
    }
    {
      dca::linalg::Matrix<ScalarType, dca::linalg::CPU> a(size_a);
      dca::linalg::Matrix<ScalarType, dca::linalg::CPU> b(size_b);
      dca::linalg::Matrix<ScalarType, dca::linalg::CPU> c(size_c);

      testing::setMatrixElements(a, val_a);
      testing::setMatrixElements(b, val_b);
      testing::setMatrixElements(c, val_c);

      dca::linalg::Matrix<ScalarType, dca::linalg::CPU> cref(c);

      ScalarType alpha = .57;
      ScalarType beta = -.712;

      testing::refGemm('N', 'N', alpha, a, b, beta, cref);
      dca::linalg::matrixop::gemm(alpha, a, b, beta, c);

      for (int j = 0; j < c.nrCols(); ++j)
        for (int i = 0; i < c.nrRows(); ++i) {
          EXPECT_NEAR(cref(i, j), c(i, j), 500 * this->epsilon);
        }
    }
  }
  {
    int m = 2;
    int k = 4;
    int n = 3;
    std::pair<int, int> size_c(2, 3);

    char trans_opts[] = {'N', 'T'};

    for (auto transa : trans_opts) {
      std::pair<int, int> size_a;
      if (transa == 'N') {
        size_a.first = m;
        size_a.second = k;
      }
      else {
        size_a.first = k;
        size_a.second = m;
      }

      for (auto transb : trans_opts) {
        std::pair<int, int> size_b;
        if (transb == 'N') {
          size_b.first = k;
          size_b.second = n;
        }
        else {
          size_b.first = n;
          size_b.second = k;
        }

        {
          dca::linalg::Matrix<ScalarType, dca::linalg::CPU> a(size_a);
          dca::linalg::Matrix<ScalarType, dca::linalg::CPU> b(size_b);
          dca::linalg::Matrix<ScalarType, dca::linalg::CPU> c(size_c);

          testing::setMatrixElements(a, val_a);
          testing::setMatrixElements(b, val_b);
          testing::setMatrixElements(c, val_c);

          dca::linalg::Matrix<ScalarType, dca::linalg::CPU> cref(c);

          testing::refGemm(transa, transb, ScalarType(1), a, b, ScalarType(0), cref);
          dca::linalg::matrixop::gemm(transa, transb, a, b, c);

          for (int j = 0; j < c.nrCols(); ++j)
            for (int i = 0; i < c.nrRows(); ++i) {
              EXPECT_NEAR(cref(i, j), c(i, j), 500 * this->epsilon);
            }
        }
        {
          dca::linalg::Matrix<ScalarType, dca::linalg::CPU> a(size_a);
          dca::linalg::Matrix<ScalarType, dca::linalg::CPU> b(size_b);
          dca::linalg::Matrix<ScalarType, dca::linalg::CPU> c(size_c);

          testing::setMatrixElements(a, val_a);
          testing::setMatrixElements(b, val_b);
          testing::setMatrixElements(c, val_c);

          dca::linalg::Matrix<ScalarType, dca::linalg::CPU> cref(c);

          ScalarType alpha = .57;
          ScalarType beta = -.712;

          testing::refGemm(transa, transb, alpha, a, b, beta, cref);
          dca::linalg::matrixop::gemm(transa, transb, alpha, a, b, beta, c);

          for (int j = 0; j < c.nrCols(); ++j)
            for (int i = 0; i < c.nrRows(); ++i) {
              EXPECT_NEAR(cref(i, j), c(i, j), 500 * this->epsilon);
            }
        }
      }
    }
  }
}

template <typename ComplexType>
class MatrixopComplexCPUTest : public ::testing::Test {
public:
  static const typename ComplexType::value_type epsilon;
};
template <typename ComplexType>
const typename ComplexType::value_type MatrixopComplexCPUTest<ComplexType>::epsilon =
    std::numeric_limits<typename ComplexType::value_type>::epsilon();

typedef ::testing::Types<std::complex<float>, std::complex<double>> ComplexFloatingPointTypes;
TYPED_TEST_CASE(MatrixopComplexCPUTest, ComplexFloatingPointTypes);

TYPED_TEST(MatrixopComplexCPUTest, Gemm) {
  using ScalarType = TypeParam;
  auto val_a = [](int i, int j) { return ScalarType(3 * i - 2 * j, 1 - i * i + j); };
  auto val_b = [](int i, int j) { return ScalarType(i - 2 * j * j, 4 * i - 3 * j); };
  auto val_c = [](int i, int j) { return ScalarType(2 * i - j, 2 * j - i); };
  {
    std::pair<int, int> size_a(2, 3);
    std::pair<int, int> size_b(3, 4);
    std::pair<int, int> size_c(2, 4);

    dca::linalg::Matrix<ScalarType, dca::linalg::CPU> a(size_a);
    dca::linalg::Matrix<ScalarType, dca::linalg::CPU> b(size_b);
    dca::linalg::Matrix<ScalarType, dca::linalg::CPU> c(size_c);
    {
      testing::setMatrixElements(a, val_a);
      testing::setMatrixElements(b, val_b);
      testing::setMatrixElements(c, val_c);

      dca::linalg::Matrix<ScalarType, dca::linalg::CPU> cref(c);

      testing::refGemm('N', 'N', ScalarType(1), a, b, ScalarType(0), cref);
      dca::linalg::matrixop::gemm(a, b, c);

      for (int j = 0; j < c.nrCols(); ++j)
        for (int i = 0; i < c.nrRows(); ++i) {
          EXPECT_GE(500 * this->epsilon, std::abs(cref(i, j) - c(i, j)));
        }
    }
    {
      dca::linalg::Matrix<ScalarType, dca::linalg::CPU> a(size_a);
      dca::linalg::Matrix<ScalarType, dca::linalg::CPU> b(size_b);
      dca::linalg::Matrix<ScalarType, dca::linalg::CPU> c(size_c);

      testing::setMatrixElements(a, val_a);
      testing::setMatrixElements(b, val_b);
      testing::setMatrixElements(c, val_c);

      dca::linalg::Matrix<ScalarType, dca::linalg::CPU> cref(c);

      ScalarType alpha = .57;
      ScalarType beta = -.712;

      testing::refGemm('N', 'N', alpha, a, b, beta, cref);
      dca::linalg::matrixop::gemm(alpha, a, b, beta, c);

      for (int j = 0; j < c.nrCols(); ++j)
        for (int i = 0; i < c.nrRows(); ++i) {
          EXPECT_GE(500 * this->epsilon, std::abs(cref(i, j) - c(i, j)));
        }
    }
  }
  {
    int m = 2;
    int k = 4;
    int n = 3;
    std::pair<int, int> size_c(2, 3);

    char trans_opts[] = {'N', 'T', 'C'};

    for (auto transa : trans_opts) {
      std::pair<int, int> size_a;
      if (transa == 'N') {
        size_a.first = m;
        size_a.second = k;
      }
      else {
        size_a.first = k;
        size_a.second = m;
      }

      for (auto transb : trans_opts) {
        std::pair<int, int> size_b;
        if (transb == 'N') {
          size_b.first = k;
          size_b.second = n;
        }
        else {
          size_b.first = n;
          size_b.second = k;
        }

        {
          dca::linalg::Matrix<ScalarType, dca::linalg::CPU> a(size_a);
          dca::linalg::Matrix<ScalarType, dca::linalg::CPU> b(size_b);
          dca::linalg::Matrix<ScalarType, dca::linalg::CPU> c(size_c);

          testing::setMatrixElements(a, val_a);
          testing::setMatrixElements(b, val_b);
          testing::setMatrixElements(c, val_c);

          dca::linalg::Matrix<ScalarType, dca::linalg::CPU> cref(c);

          testing::refGemm(transa, transb, ScalarType(1), a, b, ScalarType(0), cref);
          dca::linalg::matrixop::gemm(transa, transb, a, b, c);

          for (int j = 0; j < c.nrCols(); ++j)
            for (int i = 0; i < c.nrRows(); ++i) {
              EXPECT_GE(500 * this->epsilon, std::abs(cref(i, j) - c(i, j)));
            }
        }
        {
          dca::linalg::Matrix<ScalarType, dca::linalg::CPU> a(size_a);
          dca::linalg::Matrix<ScalarType, dca::linalg::CPU> b(size_b);
          dca::linalg::Matrix<ScalarType, dca::linalg::CPU> c(size_c);

          testing::setMatrixElements(a, val_a);
          testing::setMatrixElements(b, val_b);
          testing::setMatrixElements(c, val_c);

          dca::linalg::Matrix<ScalarType, dca::linalg::CPU> cref(c);

          ScalarType alpha = .57;
          ScalarType beta = -.712;

          testing::refGemm(transa, transb, alpha, a, b, beta, cref);
          dca::linalg::matrixop::gemm(transa, transb, alpha, a, b, beta, c);

          for (int j = 0; j < c.nrCols(); ++j)
            for (int i = 0; i < c.nrRows(); ++i) {
              EXPECT_GE(500 * this->epsilon, std::abs(cref(i, j) - c(i, j)));
            }
        }
      }
    }
  }
}

template <typename ScalarType>
class MatrixopMixedCPUTest : public ::testing::Test {
public:
  static const ScalarType epsilon;
};
template <typename ScalarType>
const ScalarType MatrixopMixedCPUTest<ScalarType>::epsilon = std::numeric_limits<ScalarType>::epsilon();

TYPED_TEST_CASE(MatrixopMixedCPUTest, FloatingPointTypes);

TYPED_TEST(MatrixopMixedCPUTest, Gemm) {
  using ScalarType = TypeParam;
  using ComplexType = std::complex<ScalarType>;
  auto val_real = [](int i, int j) { return 3 * i - 2 * j; };
  auto val_complex = [](int i, int j) { return ComplexType(i - 2 * j * j, 4 * i - 3 * j); };
  auto val_c = [](int i, int j) { return ComplexType(2 * i - j, 2 * j - i); };

  int m = 2;
  int k = 4;
  int n = 3;
  std::pair<int, int> size_c(2, 3);

  char trans_opts[] = {'N', 'T', 'C'};

  for (auto transa : trans_opts) {
    std::pair<int, int> size_a;
    if (transa == 'N') {
      size_a.first = m;
      size_a.second = k;
    }
    else {
      size_a.first = k;
      size_a.second = m;
    }

    for (auto transb : trans_opts) {
      std::pair<int, int> size_b;
      if (transb == 'N') {
        size_b.first = k;
        size_b.second = n;
      }
      else {
        size_b.first = n;
        size_b.second = k;
      }

      if (transa != 'C') {
        dca::linalg::Matrix<ScalarType, dca::linalg::CPU> a(size_a);
        dca::linalg::Matrix<ComplexType, dca::linalg::CPU> b(size_b);
        dca::linalg::Matrix<ComplexType, dca::linalg::CPU> c(size_c);

        testing::setMatrixElements(a, val_real);
        testing::setMatrixElements(b, val_complex);
        testing::setMatrixElements(c, val_c);

        dca::linalg::Matrix<ComplexType, dca::linalg::CPU> cref(c);

        testing::refGemm(transa, transb, ComplexType(1), a, b, ComplexType(0), cref);
        dca::linalg::matrixop::gemm(transa, transb, a, b, c);

        for (int j = 0; j < c.nrCols(); ++j)
          for (int i = 0; i < c.nrRows(); ++i) {
            EXPECT_GE(500 * this->epsilon, std::abs(cref(i, j) - c(i, j)));
          }
      }
      if (transb != 'C') {
        dca::linalg::Matrix<ComplexType, dca::linalg::CPU> a(size_a);
        dca::linalg::Matrix<ScalarType, dca::linalg::CPU> b(size_b);
        dca::linalg::Matrix<ComplexType, dca::linalg::CPU> c(size_c);

        testing::setMatrixElements(a, val_complex);
        testing::setMatrixElements(b, val_real);
        testing::setMatrixElements(c, val_c);

        dca::linalg::Matrix<ComplexType, dca::linalg::CPU> cref(c);

        testing::refGemm(transa, transb, ComplexType(1), a, b, ComplexType(0), cref);
        dca::linalg::matrixop::gemm(transa, transb, a, b, c);

        for (int j = 0; j < c.nrCols(); ++j)
          for (int i = 0; i < c.nrRows(); ++i) {
            EXPECT_GE(500 * this->epsilon, std::abs(cref(i, j) - c(i, j)));
          }
      }
    }
  }
}

TYPED_TEST(MatrixopRealCPUTest, Trsm) {
  using ScalarType = TypeParam;
  auto val_a = [](int i, int j) { return 1 + 3 * i - 2 * j; };
  auto val_b = [](int i, int j) { return 4 * i - 3 * j; };
  std::pair<int, int> size_a(3, 3);
  std::pair<int, int> size_b(3, 4);

  char uplo_opts[] = {'U', 'L'};
  char diag_opts[] = {'U', 'N'};

  for (auto uplo : uplo_opts) {
    for (auto diag : diag_opts) {
      int m = size_b.first;
      int n = size_b.second;
      dca::linalg::Matrix<ScalarType, dca::linalg::CPU> a(size_a);
      dca::linalg::Matrix<ScalarType, dca::linalg::CPU> b(size_b);

      testing::setMatrixElements(a, val_a);
      testing::setMatrixElements(b, val_b);

      dca::linalg::Matrix<ScalarType, dca::linalg::CPU> b_orig(b);

      dca::linalg::matrixop::trsm(uplo, diag, a, b);
      dca::linalg::blas::trmm("L", &uplo, "N", &diag, m, n, ScalarType(1), a.ptr(),
                              a.leadingDimension(), b.ptr(), b.leadingDimension());

      for (int j = 0; j < b.nrCols(); ++j)
        for (int i = 0; i < b.nrRows(); ++i) {
          EXPECT_NEAR(b_orig(i, j), b(i, j), 500 * this->epsilon);
        }
    }
  }
}

TYPED_TEST(MatrixopComplexCPUTest, Trsm) {
  using ScalarType = TypeParam;
  auto val_a = [](int i, int j) { return ScalarType(3 * i - 2 * j, 1 - i * i + j); };
  auto val_b = [](int i, int j) { return ScalarType(i - 2 * j * j, 4 * i - 3 * j); };
  std::pair<int, int> size_a(3, 3);
  std::pair<int, int> size_b(3, 4);

  char uplo_opts[] = {'U', 'L'};
  char diag_opts[] = {'U', 'N'};

  for (auto uplo : uplo_opts) {
    for (auto diag : diag_opts) {
      int m = size_b.first;
      int n = size_b.second;
      dca::linalg::Matrix<ScalarType, dca::linalg::CPU> a(size_a);
      dca::linalg::Matrix<ScalarType, dca::linalg::CPU> b(size_b);

      testing::setMatrixElements(a, val_a);
      testing::setMatrixElements(b, val_b);

      dca::linalg::Matrix<ScalarType, dca::linalg::CPU> b_orig(b);

      dca::linalg::matrixop::trsm(uplo, diag, a, b);
      dca::linalg::blas::trmm("L", &uplo, "N", &diag, m, n, ScalarType(1), a.ptr(),
                              a.leadingDimension(), b.ptr(), b.leadingDimension());

      for (int j = 0; j < b.nrCols(); ++j)
        for (int i = 0; i < b.nrRows(); ++i) {
          EXPECT_GE(500 * this->epsilon, std::abs(b_orig(i, j) - b(i, j)));
        }
    }
  }
}

TEST(MatrixopCPUTest, InsertRowCol) {
  std::pair<int, int> size2(3, 4);
  auto val = [](int i, int j) { return 10 * i + j; };
  dca::linalg::Matrix<double, dca::linalg::CPU> mat(size2);
  testing::setMatrixElements(mat, val);

  for (int ii : {0, 1, size2.first}) {
    dca::linalg::Matrix<double, dca::linalg::CPU> mat_test(mat);

    dca::linalg::matrixop::insertRow(mat_test, ii);
    for (int j = 0; j < mat.nrCols(); ++j) {
      for (int i = 0; i < ii; ++i)
        EXPECT_EQ(mat(i, j), mat_test(i, j));
      for (int i = ii; i < mat.nrRows(); ++i)
        EXPECT_EQ(mat(i, j), mat_test(i + 1, j));

      EXPECT_EQ(0, mat_test(ii, j));
    }
  }
  for (int jj : {0, 1, size2.second}) {
    dca::linalg::Matrix<double, dca::linalg::CPU> mat_test(mat);

    dca::linalg::matrixop::insertCol(mat_test, jj);
    for (int i = 0; i < mat.nrRows(); ++i) {
      for (int j = 0; j < jj; ++j)
        EXPECT_EQ(mat(i, j), mat_test(i, j));
      for (int j = jj; j < mat.nrCols(); ++j)
        EXPECT_EQ(mat(i, j), mat_test(i, j + 1));

      EXPECT_EQ(0, mat_test(i, jj));
    }
  }
}

TEST(MatrixopCPUTest, RemoveRowCol) {
  std::pair<int, int> size2(3, 4);
  auto val = [](int i, int j) { return 10 * i + j; };
  dca::linalg::Matrix<double, dca::linalg::CPU> mat(size2);
  testing::setMatrixElements(mat, val);

  for (int ii : {0, 1, size2.first - 1}) {
    dca::linalg::Matrix<double, dca::linalg::CPU> mat_test(mat);

    dca::linalg::matrixop::removeRow(mat_test, ii);
    for (int j = 0; j < mat.nrCols(); ++j) {
      for (int i = 0; i < ii; ++i)
        EXPECT_EQ(mat(i, j), mat_test(i, j));
      for (int i = ii + 1; i < mat.nrRows(); ++i)
        EXPECT_EQ(mat(i, j), mat_test(i - 1, j));
    }
  }
  for (int jj : {0, 1, size2.second - 1}) {
    dca::linalg::Matrix<double, dca::linalg::CPU> mat_test(mat);

    dca::linalg::matrixop::removeCol(mat_test, jj);
    for (int i = 0; i < mat.nrRows(); ++i) {
      for (int j = 0; j < jj; ++j)
        EXPECT_EQ(mat(i, j), mat_test(i, j));
      for (int j = jj + 1; j < mat.nrCols(); ++j)
        EXPECT_EQ(mat(i, j), mat_test(i, j - 1));
    }
  }
  for (int ii : {0, 1, size2.first - 1}) {
    for (int jj : {0, 1, size2.second - 1}) {
      dca::linalg::Matrix<double, dca::linalg::CPU> mat_test(mat);

      dca::linalg::matrixop::removeRowAndCol(mat_test, ii, jj);
      for (int j = 0; j < jj; ++j) {
        for (int i = 0; i < ii; ++i)
          EXPECT_EQ(mat(i, j), mat_test(i, j));
        for (int i = ii + 1; i < mat.nrRows(); ++i)
          EXPECT_EQ(mat(i, j), mat_test(i - 1, j));
      }
      for (int j = jj + 1; j < mat.nrCols(); ++j) {
        for (int i = 0; i < ii; ++i)
          EXPECT_EQ(mat(i, j), mat_test(i, j - 1));
        for (int i = ii + 1; i < mat.nrRows(); ++i)
          EXPECT_EQ(mat(i, j), mat_test(i - 1, j - 1));
      }
    }
  }
  for (int ii : {0, 1, std::min(size2.first, size2.second) - 1}) {
    dca::linalg::Matrix<double, dca::linalg::CPU> mat_test(mat);

    dca::linalg::matrixop::removeRowAndCol(mat_test, ii);
    for (int j = 0; j < ii; ++j) {
      for (int i = 0; i < ii; ++i)
        EXPECT_EQ(mat(i, j), mat_test(i, j));
      for (int i = ii + 1; i < mat.nrRows(); ++i)
        EXPECT_EQ(mat(i, j), mat_test(i - 1, j));
    }
    for (int j = ii + 1; j < mat.nrCols(); ++j) {
      for (int i = 0; i < ii; ++i)
        EXPECT_EQ(mat(i, j), mat_test(i, j - 1));
      for (int i = ii + 1; i < mat.nrRows(); ++i)
        EXPECT_EQ(mat(i, j), mat_test(i - 1, j - 1));
    }
  }
}