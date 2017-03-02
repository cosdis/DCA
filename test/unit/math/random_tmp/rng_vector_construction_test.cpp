// Copyright (C) 2009-2016 ETH Zurich
// Copyright (C) 2007?-2016 Center for Nanophase Materials Sciences, ORNL
// All rights reserved.
//
// See LICENSE.txt for terms of usage.
// See CITATION.txt for citation guidelines if you use this code for scientific publications.
//
// Author: Urs R. Haehner (haehneru@itp.phys.ethz.ch)
//
// This file tests the construction of random number generators in a std::vector.
// The test is put in a separate file (compilation unit) to keep control over static variables.

#include "dca/math/random/random.hpp"
#include <vector>
#include "gtest/gtest.h"

template <typename Generator>
class RandomTest : public ::testing::Test {};

using Generators = ::testing::Types<dca::math::random::StdRandomWrapper<std::ranlux48_base>>;

TYPED_TEST_CASE(RandomTest, Generators);

TYPED_TEST(RandomTest, RngVectorConstruction) {
  using RngType = TypeParam;

  const int num_rngs = 5;
  const int proc_id = 1;
  const int num_procs = 4;
  const int seed = 42;

  std::vector<RngType> rngs;

  for (int i = 0; i < num_rngs; ++i)
    rngs.emplace_back(proc_id, num_procs, seed);

  std::vector<int> expected_ids{1, 5, 9, 13, 17};

  for (int i = 0; i < rngs.size(); ++i)
    EXPECT_EQ(expected_ids[i], rngs[i].getGlobalId());
}
