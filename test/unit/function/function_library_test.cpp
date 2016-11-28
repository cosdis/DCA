// Copyright (C) 2009-2016 ETH Zurich
// Copyright (C) 2007?-2016 Center for Nanophase Materials Sciences, ORNL
// All rights reserved.
//
// See LICENSE.txt for terms of usage.
// See CITATION.txt for citation guidelines if you use this code for scientific publications.
//
// Author: John Biddiscombe (john.biddiscombe@cscs.ch)
//         Giovanni Balduzzi (gbalduzz@phys.ethz.ch)
//
// This file tests the function library.

#include "dca/function/domains.hpp"
#include "dca/function/function.hpp"

#include <cassert>
#include <complex>
#include <cstdio>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <map>
#include <sstream>
#include <vector>

#include "gtest/gtest.h"

#include "dca/util/type_list.hpp"
#include "dca/util/type_utils.hpp"
#include "test/unit/function/variable_domain.hpp"

using dca::func::dmn;
using dca::func::dmn_0;
using dca::func::dmn_variadic;

namespace dca {
namespace testing {
// dca::testing::

bool compare_to_file(const std::string& filename, const std::string& check) {
  // Open the file.
  std::ifstream known_result(filename);

  if (known_result.good()) {
    std::string contents;

    // Get the right size to reserve it.
    known_result.seekg(0, std::ios::end);
    contents.reserve(known_result.tellg());

    known_result.seekg(0, std::ios::beg);

    // Read contents into string directly.
    contents.assign((std::istreambuf_iterator<char>(known_result)), std::istreambuf_iterator<char>());

    return (contents == check);
  }

  else {
    std::ofstream new_result(filename);
    new_result << check.c_str();
    std::cout << "No baseline file exists, writing new one " << filename.c_str() << std::endl;
  }

  return false;
}

// A selection of domain types we can use for testing.
typedef dmn_0<dmn<1, double>> test_domain_0a;
typedef dmn_0<dmn<2, double>> test_domain_0b;
typedef dmn_0<dmn<4, double>> test_domain_0c;
typedef dmn_0<dmn<8, double>> test_domain_0d;
typedef dmn_variadic<test_domain_0d> test_domain_1d;
typedef dmn_variadic<test_domain_0a, test_domain_0b> test_domain_2a;
typedef dmn_variadic<test_domain_0c, test_domain_0d> test_domain_2c;
typedef dmn_variadic<test_domain_2a, test_domain_2c> test_domain_4a;
typedef dmn_variadic<test_domain_4a, test_domain_4a, test_domain_2c> test_domain_16;

typedef dmn_variadic<test_domain_0d> test_domain_0v;
typedef dmn_variadic<test_domain_4a, test_domain_4a, test_domain_2c> test_domain_16v;

dca::func::function<double, test_domain_0a> function_0a("test_domain_0a");
dca::func::function<double, test_domain_0b> function_0b("test_domain_0b");
dca::func::function<double, test_domain_0c> function_0c("test_domain_0c");
dca::func::function<double, test_domain_0d> function_0d("test_domain_0d");
dca::func::function<double, test_domain_1d> function_1d("test_domain_1d");
dca::func::function<double, test_domain_2a> function_2a("test_domain_2a");
dca::func::function<double, test_domain_4a> function_4a("test_domain_4a");
dca::func::function<double, test_domain_16> function_16("test_domain_16");

test_domain_0v dummy;
test_domain_16v dummy2;

template <typename T1>
struct function_test {};

template <int N, typename Dmn>
struct function_test<dca::func::function<double, dmn<N, Dmn>>> {
  typedef dca::func::function<double, dmn<N, Dmn>> fType;

  function_test(fType& func) : f(func) {}

  template <typename Arg>
  bool check_1(Arg /*arg*/) {
    /*
    std::cout << "Sub branch size " << std::endl;
    for (int i = 0; i < f.get_Nb_branch_domains(); i++) {
      std::cout << f.get_branch_size(i) << "\t";
    }
    std::cout << std::endl;

    std::cout << "Sub branch steps " << std::endl;
    for (int i = 0; i < f.get_Nb_branch_domains(); i++) {
      std::cout << f.get_branch_domain_steps()[i] << "\t";
    }
    std::cout << std::endl;
    */
    return true;
  }

  fType& f;
};

template <typename Domain>
struct function_test<dca::func::function<double, Domain>> {
  typedef dca::func::function<double, Domain> fType;
  typedef typename fType::this_scalar_type scalartype;
  typedef Domain domainType;
  // typedef typename Domain::this_type sub_type;

  // const int Ntypes = dca::util::Length<sub_type>::value;

  function_test(fType& func) : f(func) {}

  int signature() {
    return f.signature();
  }
  int size() {
    return f.size();
  }

  void fill_sequence() {
    int N = f.size();
    for (int i = 0; i < N; ++i) {
      f(i) = i;
      // if (i<1024) std::cout << i << ",";
    }
  }

  void check_sequence() {
    int N = f.size();
    for (int i = 0; i < N; ++i) {
      if (f(i) != i)
        throw(std::runtime_error("fault"));
    }
    // std::cout << "Ntypes " << Ntypes << " signature " << signature() << " size " << size()
    //           << std::endl;
  }

  template <typename Arg>
  bool check_1(Arg /*arg*/) {
    std::cout << "Sub branch size " << std::endl;
    for (int i = 0; i < f.get_domain().get_Nb_branch_domains(); i++) {
      std::cout << f.get_domain().get_branch_size(i) << "\t";
    }
    std::cout << std::endl;

    std::cout << "Sub branch steps " << std::endl;
    for (int i = 0; i < f.get_domain().get_Nb_branch_domains(); i++) {
      std::cout << f.get_domain().get_branch_domain_steps()[i] << "\t";
    }
    std::cout << std::endl;

    // IsSame<mp_append<test_domain_0a::this_type, test_domain_0b::this_type>::type, double>();
    // IsSame<test_domain_0a::this_type, test_domain_0b::this_type>();
    // IsSame<test_domain_0a, test_domain_0b>();
    // IsSame<test_domain_2a, test_domain_2c>();

    std::cout << std::endl;
    using test_list = dca::util::Typelist<int*, double, float&, int, int*, int, int, const float*,
                                          const int, long long, int&>;
    using test_list2 = dca::util::Typelist<float, int, int*, double*, int, int, int*, double, int,
                                           float, const int, long long, int&>;
    std::cout << "Testing Typelist Length " << dca::util::mp_size<test_list>::value << std::endl;
    std::cout << "Testing Typelist Length " << dca::util::Length<test_list>::value << std::endl;

    std::cout << "Testing Typelist NumberOf " << dca::util::mp_count<test_list, int>::value
              << std::endl;
    std::cout << "Testing Typelist NumberOf " << dca::util::NumberOf<test_list, int>::value
              << std::endl;

    std::cout << "Testing Typelist IndexOf " << dca::util::mp_index_of<long long, test_list>::value
              << std::endl;
    std::cout << "Testing Typelist IndexOf " << dca::util::IndexOf<long long, test_list>::value
              << std::endl;

    std::cout << "Testing Typelist TypeAt "
              << dca::util::type_name<dca::util::mp_element<9, test_list>::type>().c_str()
              << std::endl;
    std::cout << "Testing Typelist TypeAt "
              << dca::util::type_name<dca::util::TypeAt<9, test_list>::type>().c_str() << std::endl;

    std::cout << "Testing Typelist Append "
              << dca::util::mp_size<dca::util::mp_append<test_list, test_list2>::type>::value
              << std::endl;
    std::cout << "Testing Typelist Append "
              << dca::util::Length<dca::util::Append<test_list, test_list2>::type>::value
              << std::endl;
    std::cout << "Testing Typelist Append/Index "
              << dca::util::mp_index_of<const float*, dca::util::mp_append<test_list, test_list2>::type>::value
              << std::endl;
    std::cout << "Testing Typelist Append/Index "
              << dca::util::IndexOf<const float*, dca::util::Append<test_list, test_list2>::type>::value
              << std::endl;

    std::cout << "Testing Typelist Prepend "
              << dca::util::mp_size<dca::util::mp_prepend<test_list, test_list2>::type>::value
              << std::endl;
    std::cout << "Testing Typelist Prepend "
              << dca::util::Length<dca::util::Prepend<test_list, test_list2>::type>::value
              << std::endl;
    std::cout << "Testing Typelist Prepend/Index "
              << dca::util::mp_index_of<const float*,
                                        dca::util::mp_prepend<test_list, test_list2>::type>::value
              << std::endl;
    std::cout << "Testing Typelist Prepend/Index "
              << dca::util::IndexOf<const float*, dca::util::Prepend<test_list, test_list2>::type>::value
              << std::endl;
    std::cout << std::endl;

    std::cout << "Printing typelist test_domain_16v \n";
    dca::util::print_type<test_domain_16v>::print(std::cout);

    std::cout << "Printing typelist test_list \n";
    dca::util::print_type<test_list>::print(std::cout);
    std::cout << std::endl;

    // std::cout << "\nTesting Typelist count "
    //           << dca::util::type_name<dca::util::TypeAt<2, test_list>>().c_str() << std::endl;
    // std::cout << "\nTesting Typelist count "
    //           << dca::util::type_name<dca::util::TypeAt<2, test_list>>().c_str() << std::endl;

    // typedef typename TypeAt<typename Domain::domain_typelist_0, 0>::Result dom_0;
    // std::cout << "Getting first subdomain "
    //           << "Type Id is " << typeid(dom_0).name() << std::endl;
    // dca::func::function<double, dom_0> sub_function;
    // function_test<decltype(sub_function)> sub_domain(sub_function);
    // sub_domain.check_1(1);

    return true;
  }

  template <typename... Args>
  scalartype expand(Args... /*args*/) {
    return scalartype(0);
  }

  template <typename... Args>
  bool check_value(Args... args) {
    // if (f(args...) == arg1 * offset<f, 1> + arg2 * offset<f, 2> +) {
    // }
    return f.operator()(args...) == f(args...);
    // return check_value(args...);
  }
  fType& f;
};

}  // testing
}  // dca

TEST(Function, TestDomain4a) {
  try {
    std::cout << "Leaf indexing \n";
    int index = 0;
    for (int i0 = 0; i0 < 1; ++i0) {
      for (int i1 = 0; i1 < 2; ++i1) {
        for (int i2 = 0; i2 < 4; ++i2) {
          for (int i3 = 0; i3 < 8; ++i3) {
            std::cout << i0 << "," << i1 << "," << i2 << "," << i3 << "\n";
            dca::testing::function_4a.operator()(i0, i1, i2, i3) = index++;
            // dca::testing::function_4a.operator()(i3,i2,i1,i0) = index; bad ordering
          }
        }
      }
    }
    std::cout << "Branch indexing \n";
    index = 0;
    for (int i0 = 0; i0 < 2; ++i0) {
      for (int i1 = 0; i1 < 32; ++i1) {
        std::cout << i0 << "," << i1 << "\n";
        dca::testing::function_4a.operator()(i0, i1) = index++;
      }
    }
  }
  catch (std::string& err) {
    // Check exception.
    std::cout << "Caught " << err.c_str() << std::endl;
    FAIL();
  }
}

TEST(Function, ConstCorrectness) {
  using Function = dca::func::function<double, dca::testing::test_domain_2a>;
  Function f("foo");
  f(0, 0) = 1;

  const auto f_const(f);
  EXPECT_EQ(1, f_const(0, 0));
  EXPECT_EQ(1, f_const(0));
  EXPECT_EQ("foo", f.get_name());
  const int index[]{0, 1};
  EXPECT_EQ(0, f_const(index));

  const Function f2a("bar");
  EXPECT_EQ(0, f2a(0));
  Function f2b;
  f2b = f2a;
  // Assignment does not change the name.
  EXPECT_EQ("no name", f2b.get_name());
  // But note that the following statement is resolved as a call to the copy constructor.
  Function f2c = f2a;
  EXPECT_EQ("bar", f2c.get_name());
}

TEST(Function, MoveConstructor) {
  using Function = dca::func::function<int, dca::testing::test_domain_2a>;
  Function f("foo");
  f(0, 1) = 1;

  Function f2(std::move(f));
  EXPECT_EQ(1, f2(0, 1));
  EXPECT_EQ(0, f.size());
  EXPECT_EQ("foo", f2.get_name());

  Function f3("bar");
  f3 = std::move(f2);
  EXPECT_EQ(1, f3(0, 1));
  EXPECT_EQ(0, f2.size());
  EXPECT_EQ("bar", f3.get_name());
}

TEST(Function, Reset) {
  using VarDmn = dca::func::testing::VariableDomain;
  using Domain = dca::func::dmn_variadic<dca::testing::test_domain_0b, dca::func::dmn_0<VarDmn>>;

  VarDmn::initialize(10);

  dca::func::function<float, Domain> f;
  EXPECT_EQ(2 * 10, f.size());

  VarDmn::initialize(20);

  EXPECT_EQ(2 * 10, f.size());
  f.reset();
  EXPECT_EQ(2 * 20, f.size());
}

TEST(SetToZero, SetToZero) {
  struct MyStruct {
    int a = -1;
    int b = -1;
  } mystruct;
  std::complex<double> c(1, 1);
  double d = 1;
  unsigned short int i = 1;

  dca::func::setToZero(mystruct);
  dca::func::setToZero(c);
  dca::func::setToZero(d);
  dca::func::setToZero(i);

  EXPECT_EQ(-1, mystruct.a);
  EXPECT_EQ(std::complex<double>(0, 0), c);
  EXPECT_EQ(0, d);
  EXPECT_EQ(0, i);
}

TEST(Function, FillDomain) {
  ::testing::internal::TimeInMillis elapsed1(::testing::UnitTest::GetInstance()->elapsed_time());

  dca::testing::function_test<decltype(dca::testing::function_16)> ftest(dca::testing::function_16);

  std::cout << "Print fingerprint of function 16 \n";
  dca::testing::function_16.print_fingerprint(std::cout);

  std::cout << "ftest.fill_sequence \n";
  ftest.fill_sequence();

  std::cout << "ftest.check_sequence \n";
  ftest.check_sequence();

  std::cout << "ftest.check_1 \n";
  ftest.check_1(1);

  ::testing::internal::TimeInMillis elapsed2(::testing::UnitTest::GetInstance()->elapsed_time());

  std::cout << "Elapsed time is " << (elapsed2 - elapsed1) << std::endl;
}

TEST(Function, FingerPrint) {
  std::stringstream result;

  dca::testing::function_0a.print_fingerprint(result);
  dca::testing::function_0b.print_fingerprint(result);
  dca::testing::function_0c.print_fingerprint(result);
  dca::testing::function_0d.print_fingerprint(result);
  dca::testing::function_1d.print_fingerprint(result);
  dca::testing::function_2a.print_fingerprint(result);
  dca::testing::function_4a.print_fingerprint(result);
  dca::testing::function_16.print_fingerprint(result);

  EXPECT_TRUE(dca::testing::compare_to_file(DCA_SOURCE_DIR "/test/unit/function/fingerprint.txt",
                                            result.str()));
}

TEST(Function, PrintElements) {
  std::stringstream result;

  dca::testing::function_4a.print_elements(result);

  EXPECT_TRUE(dca::testing::compare_to_file(DCA_SOURCE_DIR "/test/unit/function/print_elements.txt",
                                            result.str()));
}

TEST(Function, to_JSON) {
  std::stringstream result;

  dca::util::print_type<dca::testing::test_domain_16::this_type>::to_JSON(std::cout);
  dca::util::print_type<dca::testing::test_domain_0a::this_type>::to_JSON(result);
  result << "\n";
  dca::util::print_type<dca::testing::test_domain_0b::this_type>::to_JSON(result);
  result << "\n";
  dca::util::print_type<dca::testing::test_domain_0c::this_type>::to_JSON(result);
  result << "\n";
  dca::util::print_type<dca::testing::test_domain_0d::this_type>::to_JSON(result);
  result << "\n";
  dca::util::print_type<dca::testing::test_domain_1d::this_type>::to_JSON(result);
  result << "\n";
  dca::util::print_type<dca::testing::test_domain_2a::this_type>::to_JSON(result);
  result << "\n";
  dca::util::print_type<dca::testing::test_domain_2c::this_type>::to_JSON(result);
  result << "\n";
  dca::util::print_type<dca::testing::test_domain_4a::this_type>::to_JSON(result);
  result << "\n";
  dca::util::print_type<dca::testing::test_domain_16::this_type>::to_JSON(result);
  result << "\n";

  EXPECT_TRUE(
      dca::testing::compare_to_file(DCA_SOURCE_DIR "/test/unit/function/json.txt", result.str()));
}
