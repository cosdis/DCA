// Copyright (C) 2009-2016 ETH Zurich
// Copyright (C) 2007?-2016 Center for Nanophase Materials Sciences, ORNL
// All rights reserved.
//
// See LICENSE.txt for terms of usage.
// See CITATION.txt for citation guidelines if you use this code for scientific publications.
//
// Author: Raffaele Solca' (rasolca@itp.phys.ethz.ch)
//
// This file tests the matrix operations with the Matrix<GPU> class.

#include "dca/linalg/matrixop.hpp"
#include <complex>
#include <stdexcept>
#include <string>
#include <utility>
#include "gtest/gtest.h"
#include "dca/linalg/matrix.hpp"
#include "dca/linalg/blas/blas3.hpp"
#include "cpu_test_util.hpp"
#include "gpu_test_util.hpp"

#include "../src/comp_library/linalg/basic_cublas_functions.h"
namespace testing {
using namespace LIN_ALG;

void initializeHandelAndStreams(int id) {
  create_cublas_handle(id);
  create_stream_handle(id);

  link_thread_stream_to_cublas_handle(id, 0);
}

void destroyHandelAndStreams(int id) {
  destroy_cublas_handle(id);
  destroy_stream_handle(id);
}
}

template <typename ScalarType>
class MatrixopRealGPUTest : public ::testing::Test {
public:
  static const ScalarType epsilon;
};
template <typename ScalarType>
const ScalarType MatrixopRealGPUTest<ScalarType>::epsilon = std::numeric_limits<ScalarType>::epsilon();

typedef ::testing::Types<float, double> FloatingPointTypes;
TYPED_TEST_CASE(MatrixopRealGPUTest, FloatingPointTypes);

TYPED_TEST(MatrixopRealGPUTest, Gemm) {
  using ScalarType = TypeParam;

  testing::initializeHandelAndStreams(0);

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

      dca::linalg::Matrix<ScalarType, dca::linalg::GPU> da(a);
      dca::linalg::Matrix<ScalarType, dca::linalg::GPU> db(b);
      dca::linalg::Matrix<ScalarType, dca::linalg::GPU> dc(c);

      // Use CPU version as reference
      dca::linalg::matrixop::gemm(a, b, c);
      dca::linalg::matrixop::gemm(da, db, dc);

      dca::linalg::Matrix<ScalarType, dca::linalg::CPU> c_test(dc);

      for (int j = 0; j < c.nrCols(); ++j)
        for (int i = 0; i < c.nrRows(); ++i) {
          EXPECT_NEAR(c(i, j), c_test(i, j), 500 * this->epsilon);
        }
    }
    {
      dca::linalg::Matrix<ScalarType, dca::linalg::CPU> a(size_a);
      dca::linalg::Matrix<ScalarType, dca::linalg::CPU> b(size_b);
      dca::linalg::Matrix<ScalarType, dca::linalg::CPU> c(size_c);

      testing::setMatrixElements(a, val_a);
      testing::setMatrixElements(b, val_b);
      testing::setMatrixElements(c, val_c);

      dca::linalg::Matrix<ScalarType, dca::linalg::GPU> da(a);
      dca::linalg::Matrix<ScalarType, dca::linalg::GPU> db(b);
      dca::linalg::Matrix<ScalarType, dca::linalg::GPU> dc(c);

      ScalarType alpha = .57;
      ScalarType beta = -.712;

      // Use CPU version as reference
      dca::linalg::matrixop::gemm(alpha, a, b, beta, c);
      dca::linalg::matrixop::gemm(alpha, da, db, beta, dc);

      dca::linalg::Matrix<ScalarType, dca::linalg::CPU> c_test(dc);

      for (int j = 0; j < c.nrCols(); ++j)
        for (int i = 0; i < c.nrRows(); ++i) {
          EXPECT_NEAR(c(i, j), c_test(i, j), 500 * this->epsilon);
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

          dca::linalg::Matrix<ScalarType, dca::linalg::GPU> da(a);
          dca::linalg::Matrix<ScalarType, dca::linalg::GPU> db(b);
          dca::linalg::Matrix<ScalarType, dca::linalg::GPU> dc(c);

          // Use CPU version as reference
          dca::linalg::matrixop::gemm(transa, transb, a, b, c);
          dca::linalg::matrixop::gemm(transa, transb, da, db, dc);

          dca::linalg::Matrix<ScalarType, dca::linalg::CPU> c_test(dc);

          for (int j = 0; j < c.nrCols(); ++j)
            for (int i = 0; i < c.nrRows(); ++i) {
              EXPECT_NEAR(c(i, j), c_test(i, j), 500 * this->epsilon);
            }
        }
        {
          dca::linalg::Matrix<ScalarType, dca::linalg::CPU> a(size_a);
          dca::linalg::Matrix<ScalarType, dca::linalg::CPU> b(size_b);
          dca::linalg::Matrix<ScalarType, dca::linalg::CPU> c(size_c);

          testing::setMatrixElements(a, val_a);
          testing::setMatrixElements(b, val_b);
          testing::setMatrixElements(c, val_c);

          dca::linalg::Matrix<ScalarType, dca::linalg::GPU> da(a);
          dca::linalg::Matrix<ScalarType, dca::linalg::GPU> db(b);
          dca::linalg::Matrix<ScalarType, dca::linalg::GPU> dc(c);

          ScalarType alpha = .57;
          ScalarType beta = -.712;

          // Use CPU version as reference
          dca::linalg::matrixop::gemm(transa, transb, alpha, a, b, beta, c);
          dca::linalg::matrixop::gemm(transa, transb, alpha, da, db, beta, dc);

          dca::linalg::Matrix<ScalarType, dca::linalg::CPU> c_test(dc);

          for (int j = 0; j < c.nrCols(); ++j)
            for (int i = 0; i < c.nrRows(); ++i) {
              EXPECT_NEAR(c(i, j), c_test(i, j), 500 * this->epsilon);
            }
        }
      }
    }
  }
  testing::destroyHandelAndStreams(0);
}

template <typename ComplexType>
class MatrixopComplexGPUTest : public ::testing::Test {
public:
  static const typename ComplexType::value_type epsilon;
};
template <typename ComplexType>
const typename ComplexType::value_type MatrixopComplexGPUTest<ComplexType>::epsilon =
    std::numeric_limits<typename ComplexType::value_type>::epsilon();

typedef ::testing::Types<std::complex<float>, std::complex<double>> ComplexFloatingPointTypes;
TYPED_TEST_CASE(MatrixopComplexGPUTest, ComplexFloatingPointTypes);

// TODO: Enable this test when MEMORY_MANAGEMENT if fixed
/*
TYPED_TEST(MatrixopComplexGPUTest, Gemm) {
  using ScalarType = TypeParam;

  testing::initializeHandelAndStreams(0);

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

      dca::linalg::Matrix<ScalarType, dca::linalg::GPU> da(a);
      dca::linalg::Matrix<ScalarType, dca::linalg::GPU> db(b);
      dca::linalg::Matrix<ScalarType, dca::linalg::GPU> dc(c);

      // Use CPU version as reference
      dca::linalg::matrixop::gemm(a, b, c);
      dca::linalg::matrixop::gemm(da, db, dc);

      dca::linalg::Matrix<ScalarType, dca::linalg::CPU> c_test(dc);

      for (int j = 0; j < c.nrCols(); ++j)
        for (int i = 0; i < c.nrRows(); ++i) {
          EXPECT_GE(500 * this->epsilon, std::abs(c(i, j) - c_test(i, j)));
        }
    }
    {
      dca::linalg::Matrix<ScalarType, dca::linalg::CPU> a(size_a);
      dca::linalg::Matrix<ScalarType, dca::linalg::CPU> b(size_b);
      dca::linalg::Matrix<ScalarType, dca::linalg::CPU> c(size_c);

      testing::setMatrixElements(a, val_a);
      testing::setMatrixElements(b, val_b);
      testing::setMatrixElements(c, val_c);

      dca::linalg::Matrix<ScalarType, dca::linalg::GPU> da(a);
      dca::linalg::Matrix<ScalarType, dca::linalg::GPU> db(b);
      dca::linalg::Matrix<ScalarType, dca::linalg::GPU> dc(c);

      ScalarType alpha = .57;
      ScalarType beta = -.712;

      // Use CPU version as reference
      dca::linalg::matrixop::gemm(alpha, a, b, beta, c);
      dca::linalg::matrixop::gemm(alpha, da, db, beta, dc);

      dca::linalg::Matrix<ScalarType, dca::linalg::CPU> c_test(dc);

      for (int j = 0; j < c.nrCols(); ++j)
        for (int i = 0; i < c.nrRows(); ++i) {
          EXPECT_GE(500 * this->epsilon, std::abs(c(i, j) - c_test(i, j)));
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

          dca::linalg::Matrix<ScalarType, dca::linalg::GPU> da(a);
          dca::linalg::Matrix<ScalarType, dca::linalg::GPU> db(b);
          dca::linalg::Matrix<ScalarType, dca::linalg::GPU> dc(c);

          // Use CPU version as reference
          dca::linalg::matrixop::gemm(transa, transb, a, b, c);
          dca::linalg::matrixop::gemm(transa, transb, da, db, dc);

          dca::linalg::Matrix<ScalarType, dca::linalg::CPU> c_test(dc);

          for (int j = 0; j < c.nrCols(); ++j)
            for (int i = 0; i < c.nrRows(); ++i) {
              EXPECT_GE(500 * this->epsilon, std::abs(c(i, j) - c_test(i, j)));
            }
        }
        {
          dca::linalg::Matrix<ScalarType, dca::linalg::CPU> a(size_a);
          dca::linalg::Matrix<ScalarType, dca::linalg::CPU> b(size_b);
          dca::linalg::Matrix<ScalarType, dca::linalg::CPU> c(size_c);

          testing::setMatrixElements(a, val_a);
          testing::setMatrixElements(b, val_b);
          testing::setMatrixElements(c, val_c);

          dca::linalg::Matrix<ScalarType, dca::linalg::GPU> da(a);
          dca::linalg::Matrix<ScalarType, dca::linalg::GPU> db(b);
          dca::linalg::Matrix<ScalarType, dca::linalg::GPU> dc(c);

          ScalarType alpha = .57;
          ScalarType beta = -.712;

          // Use CPU version as reference
          dca::linalg::matrixop::gemm(transa, transb, alpha, a, b, beta, c);
          dca::linalg::matrixop::gemm(transa, transb, alpha, da, db, beta, dc);

          dca::linalg::Matrix<ScalarType, dca::linalg::CPU> c_test(dc);

          for (int j = 0; j < c.nrCols(); ++j)
            for (int i = 0; i < c.nrRows(); ++i) {
              EXPECT_GE(500 * this->epsilon, std::abs(c(i, j) - c_test(i, j)));
            }
        }
      }
    }
  }
  testing::destroyHandelAndStreams(0);
}
*/

TYPED_TEST(MatrixopRealGPUTest, Trsm) {
  using ScalarType = TypeParam;

  testing::initializeHandelAndStreams(0);

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

      dca::linalg::Matrix<ScalarType, dca::linalg::GPU> da(a);
      dca::linalg::Matrix<ScalarType, dca::linalg::GPU> db(b);

      dca::linalg::matrixop::trsm(uplo, diag, da, db);

      dca::linalg::Matrix<ScalarType, dca::linalg::CPU> b_test(db);

      dca::linalg::blas::trmm("L", &uplo, "N", &diag, m, n, ScalarType(1), a.ptr(),
                              a.leadingDimension(), b_test.ptr(), b_test.leadingDimension());

      for (int j = 0; j < b.nrCols(); ++j)
        for (int i = 0; i < b.nrRows(); ++i) {
          EXPECT_NEAR(b(i, j), b_test(i, j), 500 * this->epsilon);
        }
    }
  }
  testing::destroyHandelAndStreams(0);
}

// TODO: Enable this test when MEMORY_MANAGEMENT if fixed
/*
TYPED_TEST(MatrixopComplexGPUTest, Trsm) {
  using ScalarType = TypeParam;

  testing::initializeHandelAndStreams(0);

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

      dca::linalg::Matrix<ScalarType, dca::linalg::GPU> da(a);
      dca::linalg::Matrix<ScalarType, dca::linalg::GPU> db(b);

      dca::linalg::matrixop::trsm(uplo, diag, da, db);

      dca::linalg::Matrix<ScalarType, dca::linalg::CPU> b_test(db);

      dca::linalg::blas::trmm("L", &uplo, "N", &diag, m, n, ScalarType(1), a.ptr(),
                              a.leadingDimension(), b_test.ptr(), b_test.leadingDimension());

      for (int j = 0; j < b.nrCols(); ++j)
        for (int i = 0; i < b.nrRows(); ++i) {
          EXPECT_GE(500 * this->epsilon, std::abs(b(i, j) - b_test(i, j)));
        }
    }
  }
  testing::destroyHandelAndStreams(0);
}
*/
TEST(MatrixopGPUTest, RemoveRowCol) {
  testing::initializeHandelAndStreams(0);

  std::pair<int, int> size2(3, 4);
  auto val = [](int i, int j) { return 10 * i + j; };
  dca::linalg::Matrix<double, dca::linalg::CPU> mat(size2);
  testing::setMatrixElements(mat, val);

  for (int ii : {0, 1, size2.first - 1}) {
    dca::linalg::Matrix<double, dca::linalg::GPU> dmat(mat);
    dca::linalg::matrixop::removeRow(dmat, ii);
    dca::linalg::Matrix<double, dca::linalg::CPU> mat_test(dmat);

    for (int j = 0; j < mat.nrCols(); ++j) {
      for (int i = 0; i < ii; ++i)
        EXPECT_EQ(mat(i, j), mat_test(i, j));
      for (int i = ii + 1; i < mat.nrRows(); ++i)
        EXPECT_EQ(mat(i, j), mat_test(i - 1, j));
    }
  }
  for (int jj : {0, 1, size2.second - 1}) {
    dca::linalg::Matrix<double, dca::linalg::GPU> dmat(mat);
    dca::linalg::matrixop::removeCol(dmat, jj);
    dca::linalg::Matrix<double, dca::linalg::CPU> mat_test(dmat);

    for (int i = 0; i < mat.nrRows(); ++i) {
      for (int j = 0; j < jj; ++j)
        EXPECT_EQ(mat(i, j), mat_test(i, j));
      for (int j = jj + 1; j < mat.nrCols(); ++j)
        EXPECT_EQ(mat(i, j), mat_test(i, j - 1));
    }
  }
  for (int ii : {0, 1, size2.first - 1}) {
    for (int jj : {0, 1, size2.second - 1}) {
      dca::linalg::Matrix<double, dca::linalg::GPU> dmat(mat);
      dca::linalg::matrixop::removeRowAndCol(dmat, ii, jj);
      dca::linalg::Matrix<double, dca::linalg::CPU> mat_test(dmat);

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
    dca::linalg::Matrix<double, dca::linalg::GPU> dmat(mat);
    dca::linalg::matrixop::removeRowAndCol(dmat, ii);
    dca::linalg::Matrix<double, dca::linalg::CPU> mat_test(dmat);

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
  testing::destroyHandelAndStreams(0);
}