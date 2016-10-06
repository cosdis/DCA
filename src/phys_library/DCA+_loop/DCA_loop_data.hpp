// Copyright (C) 2009-2016 ETH Zurich
// Copyright (C) 2007?-2016 Center for Nanophase Materials Sciences, ORNL
// All rights reserved.
//
// See LICENSE.txt for terms of usage.
// See CITATION.txt for citation guidelines if you use this code for scientific publications.
//
// Author: Peter Staar (taa@zurich.ibm.com)
//
// Description

#ifndef PHYS_LIBRARY_DCA_LOOP_DCA_LOOP_DATA_HPP
#define PHYS_LIBRARY_DCA_LOOP_DCA_LOOP_DATA_HPP

#include <complex>

#include "dca/function/domains.hpp"
#include "dca/function/function.hpp"
#include "phys_library/domains/cluster/cluster_domain.h"
#include "phys_library/domains/Quantum_domain/DCA_iteration_domain.h"
#include "phys_library/domains/Quantum_domain/electron_band_domain.h"
#include "phys_library/domains/Quantum_domain/electron_spin_domain.h"

namespace DCA {
template <typename Parameters>
class DCA_loop_data {
public:
  using expansion_dmn_t = func::dmn_0<func::dmn<32, int>>;
  using DCA_iteration_domain_type = func::dmn_0<DCA_iteration_domain>;

  using b = func::dmn_0<electron_band_domain>;
  using s = func::dmn_0<electron_spin_domain>;
  using nu = func::dmn_variadic<b, s>;  // orbital-spin index

  using k_DCA = func::dmn_0<cluster_domain<double, Parameters::lattice_type::DIMENSION, CLUSTER,
                                           MOMENTUM_SPACE, BRILLOUIN_ZONE>>;

public:
  DCA_loop_data();

  template <typename Reader>
  void read(Reader& reader);

  template <typename Writer>
  void write(Writer& reader);

public:
  func::function<double, DCA_iteration_domain_type> Gflop_per_mpi_task;
  func::function<double, DCA_iteration_domain_type> times_per_mpi_task;

  func::function<double, DCA_iteration_domain_type> thermalization_per_mpi_task;
  func::function<double, DCA_iteration_domain_type> MC_integration_per_mpi_task;

  func::function<double, DCA_iteration_domain_type> Gflops_per_mpi_task;
  func::function<double, DCA_iteration_domain_type> max_Gflops_per_mpi_task;

  func::function<double, DCA_iteration_domain_type> sign;

  func::function<double, DCA_iteration_domain_type> L2_Sigma_difference;

  func::function<double, func::dmn_variadic<nu, k_DCA, DCA_iteration_domain_type>> Sigma_zero_moment;
  func::function<double, func::dmn_variadic<nu, k_DCA, DCA_iteration_domain_type>> standard_deviation;

  func::function<std::complex<double>,
                 func::dmn_variadic<nu, nu, expansion_dmn_t, DCA_iteration_domain_type>>
      sigma_lambda;

  func::function<double, func::dmn_variadic<nu, k_DCA, DCA_iteration_domain_type>> n_k;
  func::function<double, func::dmn_variadic<nu, k_DCA, DCA_iteration_domain_type>> A_k;
  func::function<double, func::dmn_variadic<nu, DCA_iteration_domain_type>> orbital_occupancies;

  func::function<double, DCA_iteration_domain_type> density;
  func::function<double, DCA_iteration_domain_type> chemical_potential;
  func::function<double, DCA_iteration_domain_type> average_expansion_order;
};

template <typename Parameters>
DCA_loop_data<Parameters>::DCA_loop_data()
    : Gflop_per_mpi_task("Gflop_per_mpi_task"),
      times_per_mpi_task("times_per_mpi_task"),

      thermalization_per_mpi_task("thermalization_per_mpi_task"),
      MC_integration_per_mpi_task("MC_integration_per_mpi_task"),

      Gflops_per_mpi_task("Gflops_per_mpi_task"),
      max_Gflops_per_mpi_task("max_Gflops_per_mpi_task"),

      sign("sign"),
      L2_Sigma_difference("L2_Sigma_difference"),

      Sigma_zero_moment("Sigma_zero_moment"),
      standard_deviation("standard_deviation"),

      sigma_lambda("sigma_lambda"),

      n_k("n_k"),
      A_k("A_k"),
      orbital_occupancies("orbital-occupancies"),

      density("density"),
      chemical_potential("chemical-potential"),
      average_expansion_order("expansion_order") {}

template <typename Parameters>
template <typename Writer>
void DCA_loop_data<Parameters>::write(Writer& writer) {
  writer.open_group("DCA-loop-functions");

  {
    writer.execute(Gflop_per_mpi_task);
    writer.execute(times_per_mpi_task);
    writer.execute(Gflops_per_mpi_task);

    writer.execute(sign);

    writer.execute(L2_Sigma_difference);
    writer.execute(standard_deviation);

    writer.execute(chemical_potential);
    writer.execute(density);
    writer.execute(average_expansion_order);

    writer.execute(Sigma_zero_moment);

    writer.execute(n_k);
    writer.execute(A_k);
    writer.execute(orbital_occupancies);
  }

  writer.close_group();
}
}

#endif  // PHYS_LIBRARY_DCA_LOOP_DCA_LOOP_DATA_HPP
