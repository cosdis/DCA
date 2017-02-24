// Copyright (C) 2009-2016 ETH Zurich
// Copyright (C) 2007?-2016 Center for Nanophase Materials Sciences, ORNL
// All rights reserved.
//
// See LICENSE.txt for terms of usage.
// See CITATION.txt for citation guidelines if you use this code for scientific publications.
//
// Author: Peter Staar (taa@zurich.ibm.com)
//         Urs R. Haehner (haehneru@itp.phys.ethz.ch)
//         Mi Jiang (jiangmi@itp.phys.ethz.ch)
//
// This class computes the vertex function \Gamma on the lattice and determines the Bethe-Salpeter
// eigenvalues and eigenvectors.

#ifndef DCA_PHYS_DCA_ANALYSIS_BSE_SOLVER_BSE_LATTICE_SOLVER_HPP
#define DCA_PHYS_DCA_ANALYSIS_BSE_SOLVER_BSE_LATTICE_SOLVER_HPP

#include <algorithm>
#include <cassert>
#include <complex>
#include <iostream>
#include <stdexcept>
#include <vector>

#include "dca/function/domains.hpp"
#include "dca/function/function.hpp"
#include "dca/linalg/linalg.hpp"
#include "dca/math/util/comparison_methods.hpp"
#include "dca/math/util/vector_operations.hpp"
#include "dca/phys/dca_step/cluster_mapping/coarsegraining/coarsegraining_sp.hpp"
#include "dca/phys/dca_step/cluster_mapping/coarsegraining/coarsegraining_tp.hpp"
#include "dca/phys/dca_step/cluster_solver/high_temperature_series_expansion/high_temperature_series_expansion_solver.hpp"
#include "dca/phys/dca_step/lattice_mapping/lattice_mapping_sp.hpp"
#include "dca/phys/dca_step/lattice_mapping/lattice_mapping_tp.hpp"
#include "dca/phys/dca_step/symmetrization/diagrammatic_symmetries.hpp"
#include "dca/phys/dca_step/symmetrization/symmetrize.hpp"
#include "dca/phys/domains/cluster/centered_cluster_domain.hpp"
#include "dca/phys/domains/cluster/cluster_domain.hpp"
#include "dca/phys/domains/quantum/electron_band_domain.hpp"
#include "dca/phys/domains/time_and_frequency/vertex_frequency_domain.hpp"
#include "dca/util/print_time.hpp"

namespace dca {
namespace phys {
namespace analysis {
// dca::phys::analysis::

template <typename ParametersType, typename DcaDataType, typename ScalarType>
class BseLatticeSolver {
public:
  using profiler_type = typename ParametersType::profiler_type;
  using concurrency_type = typename ParametersType::concurrency_type;

  // Number of leading eigenvalues/eigenvectors to store.
  static constexpr int num_evals = 10;
  using LeadingEigDmn = func::dmn_0<func::dmn<num_evals, int>>;

  const static int N_CUBIC = 3;
  using cubic_harmonics_dmn_type = func::dmn_0<func::dmn<N_CUBIC, int>>;

  using w_VERTEX = func::dmn_0<domains::vertex_frequency_domain<domains::COMPACT>>;
  using b = func::dmn_0<domains::electron_band_domain>;
  using b_b = func::dmn_variadic<b, b>;

  using k_DCA =
      func::dmn_0<domains::cluster_domain<double, ParametersType::lattice_type::DIMENSION, domains::CLUSTER,
                                          domains::MOMENTUM_SPACE, domains::BRILLOUIN_ZONE>>;
  using k_HOST =
      func::dmn_0<domains::cluster_domain<double, ParametersType::lattice_type::DIMENSION, domains::LATTICE_SP,
                                          domains::MOMENTUM_SPACE, domains::BRILLOUIN_ZONE>>;
  using k_HOST_VERTEX =
      func::dmn_0<domains::cluster_domain<double, ParametersType::lattice_type::DIMENSION, domains::LATTICE_TP,
                                          domains::MOMENTUM_SPACE, domains::BRILLOUIN_ZONE>>;

  using host_vertex_r_cluster_type =
      domains::cluster_domain<double, ParametersType::lattice_type::DIMENSION, domains::LATTICE_TP,
                              domains::REAL_SPACE, domains::BRILLOUIN_ZONE>;
  using crystal_harmonics_expansion = domains::centered_cluster_domain<host_vertex_r_cluster_type>;
  using crystal_harmonics_expansion_dmn_t = func::dmn_0<crystal_harmonics_expansion>;

  using chi_vector_dmn_t = func::dmn_variadic<b, b, crystal_harmonics_expansion_dmn_t>;

  using LatticeEigenvectorDmn = func::dmn_variadic<b, b, k_HOST_VERTEX, w_VERTEX>;
  using crystal_eigenvector_dmn_t =
      func::dmn_variadic<b, b, crystal_harmonics_expansion_dmn_t, w_VERTEX>;
  using cubic_eigenvector_dmn_t = func::dmn_variadic<b, b, cubic_harmonics_dmn_type, w_VERTEX>;

  using HOST_matrix_dmn_t = func::dmn_variadic<LatticeEigenvectorDmn, LatticeEigenvectorDmn>;

  BseLatticeSolver(ParametersType& parameters, DcaDataType& MOMS);

  template <typename Writer>
  void write(Writer& writer);

  void computeChi0Lattice();
  template <typename ClusterMatrixDmn>
  void computeGammaLattice(
      /*const*/ func::function<std::complex<ScalarType>, ClusterMatrixDmn>& Gamma_cluster);
  void diagonalizeGammaChi0();

  auto& get_leading_eigenvalues() /*const*/ {
    return leading_eigenvalues;
  };
  auto& get_leading_eigenvectors() /*const*/ {
    return leading_eigenvectors;
  };

private:
  void initialize();

  void diagonalizeGammaChi0Symmetric();
  void diagonalizeGammaChi0Full();
  void diagonalize_folded_Gamma_chi_0();

  template <typename EvElementType>  // Element type of eigenvalues and eigenvectors.
  void recordEigenvaluesAndEigenvectors(const linalg::Vector<EvElementType, linalg::CPU>& L,
                                        const linalg::Matrix<EvElementType, linalg::CPU>& VR);
  void record_eigenvalues_and_folded_eigenvectors(
      dca::linalg::Vector<std::complex<ScalarType>, dca::linalg::CPU>& L,
      dca::linalg::Matrix<std::complex<ScalarType>, dca::linalg::CPU>& VL,
      dca::linalg::Matrix<std::complex<ScalarType>, dca::linalg::CPU>& VR);

  void compute_folded_susceptibility(
      dca::linalg::Vector<std::complex<ScalarType>, dca::linalg::CPU>& L,
      dca::linalg::Matrix<std::complex<ScalarType>, dca::linalg::CPU>& VL,
      dca::linalg::Matrix<std::complex<ScalarType>, dca::linalg::CPU>& VR);

  // Multiply the eigenvectors with a phase such that they become symmetric in the Matsubara
  // frequency, i.e. g(\omega_n) = g^*(-omega_n).
  void symmetrizeLeadingEigenvectors();

  void characterize_leading_eigenvectors();

  void print_on_shell();
  void print_on_shell_ppSC();

  ParametersType& parameters;
  concurrency_type& concurrency;

  DcaDataType& MOMS;

  func::function<std::complex<ScalarType>, HOST_matrix_dmn_t> Gamma_lattice;
  func::function<std::complex<ScalarType>, func::dmn_variadic<b_b, b_b, k_HOST_VERTEX, w_VERTEX>> chi_0_lattice;
  // Matrix in \vec{k} and \omega_n with the diagonal = chi_0_lattice.
  func::function<std::complex<ScalarType>, HOST_matrix_dmn_t> chi_0_lattice_matrix;

  func::function<std::complex<ScalarType>, LeadingEigDmn> leading_eigenvalues;
  func::function<std::complex<ScalarType>, func::dmn_variadic<LeadingEigDmn, cubic_eigenvector_dmn_t>>
      leading_symmetry_decomposition;
  func::function<std::complex<ScalarType>, func::dmn_variadic<LeadingEigDmn, LatticeEigenvectorDmn>>
      leading_eigenvectors;

  func::function<std::complex<ScalarType>, func::dmn_variadic<chi_vector_dmn_t, chi_vector_dmn_t>> chi_q;

  func::function<std::complex<ScalarType>,
                 func::dmn_variadic<k_HOST_VERTEX, crystal_harmonics_expansion_dmn_t>>
      psi_k;
  func::function<std::complex<ScalarType>,
                 func::dmn_variadic<LatticeEigenvectorDmn, crystal_eigenvector_dmn_t>>
      crystal_harmonics;

  func::function<std::complex<ScalarType>, func::dmn_variadic<k_HOST_VERTEX, cubic_harmonics_dmn_type>>
      leading_symmetry_functions;
};

template <typename ParametersType, typename DcaDataType, typename ScalarType>
BseLatticeSolver<ParametersType, DcaDataType, ScalarType>::BseLatticeSolver(
    ParametersType& parameters_ref, DcaDataType& MOMS_ref)
    : parameters(parameters_ref),
      concurrency(parameters.get_concurrency()),

      MOMS(MOMS_ref),

      Gamma_lattice("Gamma_lattice"),
      chi_0_lattice("chi_0_lattice"),
      chi_0_lattice_matrix("chi_0_lattice-matrix"),

      leading_eigenvalues("leading-eigenvalues"),
      leading_symmetry_decomposition("leading-symmetry-decomposition"),
      leading_eigenvectors("leading-eigenvectors"),

      chi_q("chi(q)"),

      psi_k("psi_k"),
      crystal_harmonics("crystal_harmonics"),

      leading_symmetry_functions("leading-symmetry-functions") {
  initialize();
}

template <typename ParametersType, typename DcaDataType, typename ScalarType>
template <typename Writer>
void BseLatticeSolver<ParametersType, DcaDataType, ScalarType>::write(Writer& writer) {
  writer.execute(leading_eigenvalues);
  writer.execute(leading_eigenvectors);

  writer.execute(leading_symmetry_decomposition);
  writer.execute(leading_symmetry_functions);

  writer.execute(Gamma_lattice);
  writer.execute(chi_0_lattice);
}

template <typename ParametersType, typename DcaDataType, typename ScalarType>
void BseLatticeSolver<ParametersType, DcaDataType, ScalarType>::initialize() {
  {
    double r_cut_off = parameters.get_projection_cut_off_radius();

    crystal_harmonics_expansion::initialize();

    std::vector<std::vector<double>> r_vecs(0);

    for (int l = 0; l < crystal_harmonics_expansion::get_size(); l++)
      if (math::util::l2Norm(crystal_harmonics_expansion::get_elements()[l]) < r_cut_off)
        r_vecs.push_back(crystal_harmonics_expansion::get_elements()[l]);

    sort(r_vecs.begin(), r_vecs.end(), math::util::hasSmallerNorm<double>);

    crystal_harmonics_expansion::get_size() = r_vecs.size();
    crystal_harmonics_expansion::get_elements() = r_vecs;

    if (concurrency.id() == concurrency.last()) {
      std::cout << "\n\n\t crystal-vectors : \n";
      for (int l = 0; l < crystal_harmonics_expansion::get_size(); l++) {
        std::cout << "\t" << l << "\t";
        math::util::print(crystal_harmonics_expansion::get_elements()[l]);
        std::cout << std::endl;
      }
    }
  }

  {
    psi_k.reset();
    crystal_harmonics.reset();

    const std::complex<double> I(0, 1);

    for (int l = 0; l < crystal_harmonics_expansion::get_size(); l++) {
      std::vector<double> r_vec = crystal_harmonics_expansion::get_elements()[l];

      for (int k_ind = 0; k_ind < k_HOST_VERTEX::dmn_size(); k_ind++)
        psi_k(k_ind, l) =
            std::exp(I * math::util::innerProduct(r_vec, k_HOST_VERTEX::get_elements()[k_ind])) /
            std::sqrt(double(k_HOST_VERTEX::dmn_size()));

      for (int w_ind = 0; w_ind < w_VERTEX::dmn_size(); w_ind++)
        for (int k_ind = 0; k_ind < k_HOST_VERTEX::dmn_size(); k_ind++)
          for (int m_ind = 0; m_ind < b::dmn_size(); m_ind++)
            for (int n_ind = 0; n_ind < b::dmn_size(); n_ind++)
              crystal_harmonics(n_ind, m_ind, k_ind, w_ind, n_ind, m_ind, l, w_ind) = psi_k(k_ind, l);
    }
  }

  {
    leading_symmetry_functions.reset();
    leading_symmetry_decomposition.reset();

    for (int l = 0; l < cubic_harmonics_dmn_type::dmn_size(); l++) {
      std::complex<double> norm = 0;

      for (int k_ind = 0; k_ind < k_HOST_VERTEX::dmn_size(); k_ind++) {
        std::complex<double> value = 0;

        double kx = k_HOST_VERTEX::get_elements()[k_ind][0];
        double ky = k_HOST_VERTEX::get_elements()[k_ind][1];

        switch (l) {
          case 0:  // s-wave
            value = 1;
            break;

          case 1:  // p-wave
            value = cos(kx) + cos(ky);
            break;

          case 2:  // d-wave
            value = cos(kx) - cos(ky);
            break;

          default:
            throw std::logic_error(__FUNCTION__);
        }

        norm += conj(value) * value;

        leading_symmetry_functions(k_ind, l) = value;
      }

      for (int k_ind = 0; k_ind < k_HOST_VERTEX::dmn_size(); k_ind++)
        leading_symmetry_functions(k_ind, l) /= std::sqrt(1.e-16 + real(norm));
    }
  }
}

template <typename ParametersType, typename DcaDataType, typename ScalarType>
void BseLatticeSolver<ParametersType, DcaDataType, ScalarType>::computeChi0Lattice() {
  profiler_type prof(__FUNCTION__, "BseLatticeSolver", __LINE__);

  if (concurrency.id() == concurrency.first())
    std::cout << "\n" << __FUNCTION__ << std::endl;

  using HTS_solver_type =
      solver::HighTemperatureSeriesExpansionSolver<dca::linalg::CPU, ParametersType, DcaDataType>;
  using lattice_map_sp_type = latticemapping::lattice_mapping_sp<ParametersType, k_DCA, k_HOST>;
  using coarsegraining_sp_type = clustermapping::coarsegraining_sp<ParametersType, k_DCA>;
  using coarsegraining_tp_type = clustermapping::coarsegraining_tp<ParametersType, k_HOST_VERTEX>;

  coarsegraining_tp_type coarsegraining_tp_obj(parameters);

  // DCA+: Compute \chi_0 from continuous lattice self-energy.
  if (parameters.do_dca_plus()) {
    lattice_map_sp_type lattice_mapping_obj(parameters);

    MOMS.Sigma_lattice_interpolated = 0.;
    MOMS.Sigma_lattice_coarsegrained = 0.;
    MOMS.Sigma_lattice = 0.;

    if (parameters.hts_approximation()) {
      coarsegraining_sp_type coarsegraining_sp_obj(parameters);

      DcaDataType MOMS_HTS(parameters);
      MOMS_HTS.H_HOST = MOMS.H_HOST;
      MOMS_HTS.H_interactions = MOMS.H_interactions;

      HTS_solver_type HTS_solver(parameters, MOMS_HTS);

      lattice_mapping_obj.execute_with_HTS_approximation(
          MOMS_HTS, HTS_solver, coarsegraining_sp_obj, MOMS.Sigma, MOMS.Sigma_lattice_interpolated,
          MOMS.Sigma_lattice_coarsegrained, MOMS.Sigma_lattice);
    }
    else {
      lattice_mapping_obj.execute(MOMS.Sigma, MOMS.Sigma_lattice_interpolated,
                                  MOMS.Sigma_lattice_coarsegrained, MOMS.Sigma_lattice);
    }
    coarsegraining_tp_obj.execute(MOMS.H_HOST, MOMS.Sigma_lattice, chi_0_lattice);
  }

  // (Standard) DCA: Compute \chi_0 from cluster self-energy.
  else {
    coarsegraining_tp_obj.execute(MOMS.H_HOST, MOMS.Sigma, chi_0_lattice);
  }

  // Renormalize and set diagonal \chi_0 matrix.
  const ScalarType renorm = 1. / (parameters.get_beta() * k_HOST_VERTEX::dmn_size());

  for (int w_ind = 0; w_ind < w_VERTEX::dmn_size(); w_ind++)
    for (int K_ind = 0; K_ind < k_HOST_VERTEX::dmn_size(); K_ind++)
      for (int m2 = 0; m2 < b::dmn_size(); m2++)
        for (int n2 = 0; n2 < b::dmn_size(); n2++)
          for (int m1 = 0; m1 < b::dmn_size(); m1++)
            for (int n1 = 0; n1 < b::dmn_size(); n1++) {
              chi_0_lattice(n1, m1, n2, m2, K_ind, w_ind) *= renorm;
              chi_0_lattice_matrix(n1, m1, K_ind, w_ind, n2, m2, K_ind, w_ind) =
                  chi_0_lattice(n1, m1, n2, m2, K_ind, w_ind);
            }

  // if (concurrency.id() == concurrency.first())
  //   std::cout << "Symmetrize \chi_{0,lattice} according to the symmetry group." << std::endl;
  // symmetrize::execute(chi_0_lattice_matrix, parameters.get_four_point_momentum_transfer());
}

template <typename ParametersType, typename DcaDataType, typename ScalarType>
template <typename ClusterMatrixDmn>
void BseLatticeSolver<ParametersType, DcaDataType, ScalarType>::computeGammaLattice(
    /*const*/ func::function<std::complex<ScalarType>, ClusterMatrixDmn>& Gamma_cluster) {
  profiler_type prof(__FUNCTION__, "BseLatticeSolver", __LINE__);

  if (concurrency.id() == concurrency.first())
    std::cout << "\n" << __FUNCTION__ << std::endl;

  // DCA+: Compute Gamma_lattice from an interpolation of Gamma_cluster followed by a deconvolution.
  if (parameters.do_dca_plus()) {
    latticemapping::lattice_mapping_tp<ParametersType, k_DCA, k_HOST_VERTEX> lattice_map_tp_obj(
        parameters);
    lattice_map_tp_obj.execute(Gamma_cluster, Gamma_lattice);
  }
  // (Standard) DCA: Simply copy Gamma_cluster.
  else {
    assert(Gamma_lattice.size() == Gamma_cluster.size());
    for (std::size_t i = 0; i < Gamma_cluster.size(); ++i)
      Gamma_lattice(i) = Gamma_cluster(i);
  }

  if (parameters.symmetrize_Gamma()) {
    if (concurrency.id() == concurrency.first())
      std::cout << "Symmetrize Gamma_lattice according to the symmetry group." << std::endl;
    symmetrize::execute(Gamma_lattice, parameters.get_four_point_momentum_transfer());

    if (concurrency.id() == concurrency.first())
      std::cout << "Symmetrize Gamma_lattice according to diagrammatic symmetries." << std::endl;
    diagrammatic_symmetries<ParametersType> diagrammatic_symmetries_obj(parameters);
    diagrammatic_symmetries_obj.execute(Gamma_lattice);
  }
}

template <typename ParametersType, typename DcaDataType, typename ScalarType>
void BseLatticeSolver<ParametersType, DcaDataType, ScalarType>::diagonalizeGammaChi0() {
  if (parameters.project_onto_crystal_harmonics()) {
    diagonalize_folded_Gamma_chi_0();
  }
  else {
#ifndef DCA_ANALYSIS_TEST_WITH_FULL_DIAGONALIZATION
    // Diagonalize the symmetric matrix \sqrt{\chi_0}\Gamma\sqrt{\chi_0}.
    // The origin in momentum space has always index = 0.
    if (parameters.get_four_point_type() == PARTICLE_PARTICLE_UP_DOWN &&
        parameters.get_four_point_momentum_transfer_index() == 0 &&
        parameters.get_four_point_frequency_transfer() == 0) {
      diagonalizeGammaChi0Symmetric();
    }
    else
#endif  // DCA_ANALYSIS_TEST_WITH_FULL_DIAGONALIZATION
      diagonalizeGammaChi0Full();
  }
  characterize_leading_eigenvectors();
}

template <typename ParametersType, typename DcaDataType, typename ScalarType>
void BseLatticeSolver<ParametersType, DcaDataType, ScalarType>::diagonalizeGammaChi0Symmetric() {
  profiler_type prof(__FUNCTION__, "BseLatticeSolver", __LINE__);

  if (concurrency.id() == concurrency.first())
    std::cout << "\n" << __FUNCTION__ << std::endl;

  const int N = LatticeEigenvectorDmn::dmn_size();

  linalg::Matrix<std::complex<ScalarType>, linalg::CPU> Gamma("Gamma", N);
  linalg::Matrix<std::complex<ScalarType>, linalg::CPU> chi_0("chi_0",
                                                              N);  // Actually \sqrt{\chi_0}.
  linalg::Matrix<std::complex<ScalarType>, linalg::CPU> chi_0_Gamma("chi_0_Gamma", N);
  linalg::Matrix<std::complex<ScalarType>, linalg::CPU> chi_0_Gamma_chi_0_complex(
      "chi_0_Gamma_chi_0_complex", N);
  linalg::Matrix<ScalarType, linalg::CPU> chi_0_Gamma_chi_0_real("chi_0_Gamma_chi_0_real", N);

  if (concurrency.id() == concurrency.first())
    std::cout << "Compute sqrt{chi_0}*Gamma*sqrt{chi_0}: " << util::print_time() << std::endl;

  // Copy functions into matrices.
  for (int j = 0; j < N; j++)
    for (int i = 0; i < N; i++)
      Gamma(i, j) = Gamma_lattice(i, j);

  for (int j = 0; j < N; j++)
    for (int i = 0; i < N; i++)
      chi_0(i, j) = std::sqrt(chi_0_lattice_matrix(i, j));

  // Compute \sqrt{\chi_0}\Gamma\sqrt{\chi_0}.
  linalg::matrixop::gemm(chi_0, Gamma, chi_0_Gamma);
  linalg::matrixop::gemm(chi_0_Gamma, chi_0, chi_0_Gamma_chi_0_complex);

  // \sqrt{\chi_0}\Gamma\sqrt{\chi_0} is real.
  for (int j = 0; j < N; j++)
    for (int i = 0; i < N; i++) {
      assert(std::imag(chi_0_Gamma_chi_0_complex(i, j)) < 1.e-6);
      chi_0_Gamma_chi_0_real(i, j) = std::real(chi_0_Gamma_chi_0_complex(i, j));
    }

  if (concurrency.id() == concurrency.first()) {
    std::cout << "Finished: " << util::print_time() << std::endl;
    std::cout << "Diagonalize sqrt{chi_0}*Gamma*sqrt{chi_0}: " << util::print_time() << std::endl;
  }

  linalg::Vector<ScalarType, linalg::CPU> L("L (BseLatticeSolver)", N);
  linalg::Matrix<ScalarType, linalg::CPU> VR("VR (BseLatticeSolver)", N);

  // Diagonalize the real and symmetric matrix \sqrt{\chi_0}\Gamma\sqrt{\chi_0}.
  linalg::matrixop::eigensolverHermitian('V', 'U', chi_0_Gamma_chi_0_real, L, VR);

  if (concurrency.id() == concurrency.first())
    std::cout << "Finished: " << util::print_time() << std::endl;

  // Some post-processing.
  recordEigenvaluesAndEigenvectors(L, VR);
  print_on_shell_ppSC();
}

template <typename ParametersType, typename DcaDataType, typename ScalarType>
void BseLatticeSolver<ParametersType, DcaDataType, ScalarType>::diagonalizeGammaChi0Full() {
  profiler_type prof(__FUNCTION__, "BseLatticeSolver", __LINE__);

  if (concurrency.id() == concurrency.first())
    std::cout << "\n" << __FUNCTION__ << std::endl;

  const int N = LatticeEigenvectorDmn::dmn_size();

  if (concurrency.id() == concurrency.first())
    std::cout << "Compute Gamma*chi_0: " << util::print_time() << std::endl;

  linalg::Matrix<std::complex<ScalarType>, linalg::CPU> Gamma("Gamma", N);
  linalg::Matrix<std::complex<ScalarType>, linalg::CPU> chi_0("chi_0", N);
  linalg::Matrix<std::complex<ScalarType>, linalg::CPU> Gamma_chi_0("Gamma_chi_0", N);

  // Copy functions into matrices.
  for (int j = 0; j < N; j++)
    for (int i = 0; i < N; i++)
      Gamma(i, j) = Gamma_lattice(i, j);

  for (int j = 0; j < N; j++)
    for (int i = 0; i < N; i++)
      chi_0(i, j) = chi_0_lattice_matrix(i, j);

  // Compute \Gamma\chi_0.
  linalg::matrixop::gemm(Gamma, chi_0, Gamma_chi_0);

  if (concurrency.id() == concurrency.first()) {
    std::cout << "Finished: " << util::print_time() << std::endl;
    std::cout << "Diagonalize Gamma*chi_0: " << util::print_time() << std::endl;
  }

  linalg::Vector<std::complex<ScalarType>, linalg::CPU> L("L (BseLatticeSolver)", N);
  linalg::Matrix<std::complex<ScalarType>, linalg::CPU> VR("VR (BseLatticeSolver)", N);
  linalg::Matrix<std::complex<ScalarType>, linalg::CPU> VL("VL (BseLatticeSolver)", N);

  // Diagonalize \Gamma\chi_0.
  linalg::matrixop::eigensolver('N', 'V', Gamma_chi_0, L, VL, VR);

  if (concurrency.id() == concurrency.first())
    std::cout << "Finished: " << util::print_time() << std::endl;

  // Some post-processing.
  recordEigenvaluesAndEigenvectors(L, VR);
  symmetrizeLeadingEigenvectors();

  print_on_shell();
}

template <typename ParametersType, typename DcaDataType, typename ScalarType>
template <typename EvElementType>
void BseLatticeSolver<ParametersType, DcaDataType, ScalarType>::recordEigenvaluesAndEigenvectors(
    const linalg::Vector<EvElementType, linalg::CPU>& l,
    const linalg::Matrix<EvElementType, linalg::CPU>& vr) {
  assert(vr.is_square());
  assert(l.size() == vr.size().first);

  const int size = l.size();

  // Store all eigenvalues together with their index in the vector l.
  std::vector<std::pair<EvElementType, int>> evals_index(size);
  for (int i = 0; i < size; i++) {
    evals_index[i].first = l[i];
    evals_index[i].second = i;
  }

  // Sort the eigenvalues according to their distance to 1 (closest first).
  std::stable_sort(evals_index.begin(), evals_index.end(),
                   math::util::susceptibilityPairLess<EvElementType, int>);

  // Copy the leading eigenvalues, i.e. those that are the closest to 1, and the corresponding
  // eigenvectors.
  for (int i = 0; i < num_evals; i++) {
    int index = evals_index[i].second;

    leading_eigenvalues(i) = l[index];

    for (int j = 0; j < size; j++)
      leading_eigenvectors(i, j) = vr(j, index);
  }
}

template <typename ParametersType, typename DcaDataType, typename ScalarType>
void BseLatticeSolver<ParametersType, DcaDataType, ScalarType>::diagonalize_folded_Gamma_chi_0() {
  if (concurrency.id() == concurrency.last())
    std::cout << __FUNCTION__ << std::endl;

  profiler_type prof(__FUNCTION__, "BseLatticeSolver", __LINE__);

  int N = LatticeEigenvectorDmn::dmn_size();
  int M = crystal_eigenvector_dmn_t::dmn_size();

  dca::linalg::Matrix<std::complex<ScalarType>, dca::linalg::CPU> Gamma_chi_0_crystal("Gamma_chi_0",
                                                                                      M);

  {
    dca::linalg::Matrix<std::complex<ScalarType>, dca::linalg::CPU> Gamma_chi_0_lattice(
        "Gamma_chi_0", N);

    {
      if (concurrency.id() == concurrency.last())
        std::cout << "\n\n\t compute Gamma_chi_0_lattice " << dca::util::print_time() << " ...";

      dca::linalg::Matrix<std::complex<ScalarType>, dca::linalg::CPU> Gamma("Gamma", N);
      dca::linalg::Matrix<std::complex<ScalarType>, dca::linalg::CPU> chi_0("chi_0", N);

      for (int j = 0; j < N; j++)
        for (int i = 0; i < N; i++)
          Gamma(i, j) = Gamma_lattice(i, j);

      for (int j = 0; j < N; j++)
        for (int i = 0; i < N; i++)
          chi_0(i, j) = chi_0_lattice_matrix(i, j);

      dca::linalg::matrixop::gemm(Gamma, chi_0, Gamma_chi_0_lattice);

      if (concurrency.id() == concurrency.last())
        std::cout << " finished " << dca::util::print_time() << "\n";
    }

    {
      if (concurrency.id() == concurrency.last())
        std::cout << "\n\n\t compute P_Gamma_chi_0_lattice_P " << dca::util::print_time() << " ...";

      dca::linalg::Matrix<std::complex<ScalarType>, dca::linalg::CPU> P("P",
                                                                        std::pair<int, int>(N, M));
      dca::linalg::Matrix<std::complex<ScalarType>, dca::linalg::CPU> tmp(
          "tmp", std::pair<int, int>(N, M));

      for (int j = 0; j < M; j++)
        for (int i = 0; i < N; i++)
          P(i, j) = crystal_harmonics(i, j);

      dca::linalg::matrixop::gemm('N', 'N', Gamma_chi_0_lattice, P, tmp);
      dca::linalg::matrixop::gemm('C', 'N', P, tmp, Gamma_chi_0_crystal);

      if (concurrency.id() == concurrency.last())
        std::cout << " finished " << dca::util::print_time() << "\n";
    }
  }

  {
    if (concurrency.id() == concurrency.last())
      std::cout << "\n\n\t diagonalize P_Gamma_chi_0_lattice_P " << dca::util::print_time()
                << " ...";

    dca::linalg::Vector<std::complex<ScalarType>, dca::linalg::CPU> L("L (BseLatticeSolver)", M);

    dca::linalg::Matrix<std::complex<ScalarType>, dca::linalg::CPU> VL("VL (BseLatticeSolver)", M);
    dca::linalg::Matrix<std::complex<ScalarType>, dca::linalg::CPU> VR("VR (BseLatticeSolver)", M);

    dca::linalg::matrixop::eigensolver('N', 'V', Gamma_chi_0_crystal, L, VL, VR);

    if (concurrency.id() == concurrency.last())
      std::cout << " finished " << dca::util::print_time() << "\n";

    record_eigenvalues_and_folded_eigenvectors(L, VL, VR);

    print_on_shell();
  }
}

template <typename ParametersType, typename DcaDataType, typename ScalarType>
void BseLatticeSolver<ParametersType, DcaDataType, ScalarType>::record_eigenvalues_and_folded_eigenvectors(
    dca::linalg::Vector<std::complex<ScalarType>, dca::linalg::CPU>& L,
    dca::linalg::Matrix<std::complex<ScalarType>, dca::linalg::CPU>& /*VL*/,
    dca::linalg::Matrix<std::complex<ScalarType>, dca::linalg::CPU>& VR) {
  int N = LatticeEigenvectorDmn::dmn_size();
  int M = crystal_eigenvector_dmn_t::dmn_size();

  std::vector<std::pair<std::complex<ScalarType>, int>> eigenvals(M);

  for (int i = 0; i < M; i++) {
    eigenvals[i].first = L[i];
    eigenvals[i].second = i;
  }

  stable_sort(eigenvals.begin(), eigenvals.end(),
              math::util::susceptibilityPairLess<std::complex<ScalarType>, int>);

  for (int i = 0; i < num_evals; i++) {
    int index = eigenvals[i].second;

    leading_eigenvalues(i) = L[index];

    for (int j = 0; j < N; j++)
      for (int l = 0; l < M; l++)
        leading_eigenvectors(i, j) += crystal_harmonics(j, l) * VR(l, index);
  }

  symmetrizeLeadingEigenvectors();
}

template <typename ParametersType, typename DcaDataType, typename ScalarType>
void BseLatticeSolver<ParametersType, DcaDataType, ScalarType>::compute_folded_susceptibility(
    dca::linalg::Vector<std::complex<ScalarType>, dca::linalg::CPU>& L,
    dca::linalg::Matrix<std::complex<ScalarType>, dca::linalg::CPU>& VL,
    dca::linalg::Matrix<std::complex<ScalarType>, dca::linalg::CPU>& VR) {
  int N = LatticeEigenvectorDmn::dmn_size();
  int M = crystal_eigenvector_dmn_t::dmn_size();

  dca::linalg::Matrix<std::complex<ScalarType>, dca::linalg::CPU> P_1_min_Gamma_chi_0_P(
      "P_1_min_Gamma_chi_0_P", M);
  {
    if (concurrency.id() == concurrency.last())
      std::cout << "\n\n\t invert VR  " << dca::util::print_time() << " ...";

    dca::linalg::Matrix<std::complex<ScalarType>, dca::linalg::CPU> D_inv("D_inv", M);
    dca::linalg::Matrix<std::complex<ScalarType>, dca::linalg::CPU> VR_D_inv("VR_D_inv", M);

    for (int l = 0; l < M; l++)
      D_inv(l, l) = 1. / (1. - L[l]);

    VL = VR;
    dca::linalg::matrixop::inverse(VL);

    dca::linalg::matrixop::gemm('N', 'N', VR, D_inv, VR_D_inv);
    dca::linalg::matrixop::gemm('N', 'N', VR_D_inv, VL, P_1_min_Gamma_chi_0_P);

    if (concurrency.id() == concurrency.last())
      std::cout << " finished " << dca::util::print_time() << "\n";
  }

  dca::linalg::Matrix<std::complex<ScalarType>, dca::linalg::CPU> P_chi_0_P("P_chi_0_P", M);
  {
    if (concurrency.id() == concurrency.last())
      std::cout << "\n\n\t compute P_chi_0_P " << dca::util::print_time() << " ...";

    dca::linalg::Matrix<std::complex<ScalarType>, dca::linalg::CPU> chi_0("chi_0", N);

    for (int j = 0; j < N; j++)
      for (int i = 0; i < N; i++)
        chi_0(i, j) = chi_0_lattice_matrix(i, j);

    dca::linalg::Matrix<std::complex<ScalarType>, dca::linalg::CPU> P("P", std::pair<int, int>(N, M));
    dca::linalg::Matrix<std::complex<ScalarType>, dca::linalg::CPU> tmp("tmp",
                                                                        std::pair<int, int>(N, M));

    for (int j = 0; j < M; j++)
      for (int i = 0; i < N; i++)
        P(i, j) = crystal_harmonics(i, j);

    dca::linalg::matrixop::gemm('N', 'N', chi_0, P, tmp);
    dca::linalg::matrixop::gemm('C', 'N', P, tmp, P_chi_0_P);

    if (concurrency.id() == concurrency.last())
      std::cout << " finished " << dca::util::print_time() << "\n";
  }

  {
    func::function<std::complex<ScalarType>,
                   func::dmn_variadic<crystal_eigenvector_dmn_t, crystal_eigenvector_dmn_t>>
        chi_q_tmp("chi_q_tmp");

    {
      dca::linalg::Matrix<std::complex<ScalarType>, dca::linalg::CPU> chi_matrix("chi", M);

      dca::linalg::matrixop::gemm('N', 'N', P_chi_0_P, P_1_min_Gamma_chi_0_P, chi_matrix);

      for (int j = 0; j < M; j++)
        for (int i = 0; i < M; i++)
          chi_q_tmp(i, j) = chi_matrix(i, j);
    }

    chi_q = 0.;

    for (int w2 = 0; w2 < w_VERTEX::dmn_size(); w2++)
      for (int K2 = 0; K2 < M; K2++)
        for (int m2 = 0; m2 < b::dmn_size(); m2++)
          for (int n2 = 0; n2 < b::dmn_size(); n2++)

            for (int w1 = 0; w1 < w_VERTEX::dmn_size(); w1++)
              for (int K1 = 0; K1 < M; K1++)
                for (int m1 = 0; m1 < b::dmn_size(); m1++)
                  for (int n1 = 0; n1 < b::dmn_size(); n1++)
                    chi_q(n1, m1, K1, n2, m2, K2) += chi_q_tmp(n1, m1, K1, w1, n2, m2, K2, w2);
  }

  if (concurrency.id() == concurrency.last()) {
    std::cout << "\n\n\t real(chi_q) \n\n";
    for (int i = 0; i < M; i++) {
      for (int j = 0; j < M; j++)
        std::cout << real(chi_q(i, j)) << "\t";
      std::cout << "\n";
    }
    std::cout << "\n";

    std::cout << "\n\n\t imag(chi_q) \n\n";
    for (int i = 0; i < M; i++) {
      for (int j = 0; j < M; j++)
        std::cout << imag(chi_q(i, j)) << "\t";
      std::cout << "\n";
    }
    std::cout << "\n";
  }
}

template <typename ParametersType, typename DcaDataType, typename ScalarType>
void BseLatticeSolver<ParametersType, DcaDataType, ScalarType>::print_on_shell() {
  if (concurrency.id() == concurrency.last())
    std::cout << __FUNCTION__ << std::endl;

  int N = k_HOST_VERTEX::dmn_size();
  int M = crystal_harmonics_expansion_dmn_t::dmn_size();

  if (concurrency.id() == concurrency.last()) {
    std::cout.precision(6);
    std::cout << std::scientific;

    {
      std::cout << "\n\n\t\t leading eigenvalues : ( T=" << 1. / parameters.get_beta() << " )\n\n";
      for (int i = 0; i < num_evals; i++)
        std::cout << "\t" << i << "\t[" << real(leading_eigenvalues(i)) << ", "
                  << imag(leading_eigenvalues(i)) << "]\n";
    }

    {
      std::cout << "\n\n\t\t leading eigenvectors : \n\n";

      {
        std::cout << "\t" << -1 << "\t[ " << 0. << ", " << 0. << "]";
        for (int i = 0; i < num_evals; i++)
          std::cout << "\t[ " << real(leading_eigenvalues(i)) << ", "
                    << imag(leading_eigenvalues(i)) << "]";
        std::cout << "\n\t========================================\n";
      }

      for (int l = 0; l < M; l++) {
        std::cout << "\t" << l << "\t[ " << crystal_harmonics_expansion::get_elements()[l][0]
                  << ", " << crystal_harmonics_expansion::get_elements()[l][1] << "]";

        for (int i = 0; i < num_evals; i++) {
          std::complex<ScalarType> result = 0;
          std::complex<ScalarType> norm = 0;

          for (int j = 0; j < N; j++) {
            result += conj(psi_k(j, l)) * leading_eigenvectors(i, 0, 0, j, w_VERTEX::dmn_size() / 2);
            norm += conj(leading_eigenvectors(i, 0, 0, j, w_VERTEX::dmn_size() / 2)) *
                    leading_eigenvectors(i, 0, 0, j, w_VERTEX::dmn_size() / 2);
          }

          std::cout << "\t[ " << real(result / std::sqrt(real(norm) + 1.e-16)) << ", "
                    << imag(result / std::sqrt(real(norm) + 1.e-16)) << "]";
        }

        std::cout << "\n";
      }
    }
  }
}

template <typename ParametersType, typename DcaDataType, typename ScalarType>
void BseLatticeSolver<ParametersType, DcaDataType, ScalarType>::print_on_shell_ppSC() {
  if (concurrency.id() == concurrency.last())
    std::cout << __FUNCTION__ << std::endl;

  int N = k_HOST_VERTEX::dmn_size();

  if (concurrency.id() == concurrency.last()) {
    std::cout.precision(6);
    std::cout << std::scientific;
    std::cout << "\n\n\t\t 10 leading eigenvalues : ( T=" << 1. / parameters.get_beta() << " )\n\n";
    for (int i = 0; i < num_evals; i++)
      std::cout << "\t" << i << "\t[" << leading_eigenvalues(i).real() << "]\n";

    {
      int ind0pi = 0;
      int indpi0 = 0;
      // record the index for k=[0,pi] and [pi,0]
      for (int k_ind = 0; k_ind < N; k_ind++) {
        double kx = k_HOST_VERTEX::get_elements()[k_ind][0];
        double ky = k_HOST_VERTEX::get_elements()[k_ind][1];
        if (std::abs(kx) < 1.e-3 && std::abs(ky - 3.141592) < 1.e-3) {
          ind0pi = k_ind;
        }
        if (std::abs(ky) < 1.e-3 && std::abs(kx - 3.141592) < 1.e-3) {
          indpi0 = k_ind;
        }
      }

      std::cout << "\n\n\t\t Phi_(:,k) for 10 leading eigenvectors: \n\n";
      std::cout << "  w         k=[" << k_HOST_VERTEX::get_elements()[ind0pi][0] << ", "
                << k_HOST_VERTEX::get_elements()[ind0pi][1] << "]         k=["
                << k_HOST_VERTEX::get_elements()[indpi0][0] << ", "
                << k_HOST_VERTEX::get_elements()[indpi0][1] << "] \n\n";
      for (int i = 0; i < num_evals; i++) {
        for (int w = 0; w < w_VERTEX::dmn_size(); w++) {
          std::cout << i << "   " << w_VERTEX::get_elements()[w] << "   "
                    << leading_eigenvectors(i, 0, 0, ind0pi, w).real() << "   "
                    << leading_eigenvectors(i, 0, 0, indpi0, w).real() << "\n";
        }
        std::cout << "----------------------------------------------------------------------"
                  << "\n";
      }
    }
  }
}

template <typename ParametersType, typename DcaDataType, typename ScalarType>
void BseLatticeSolver<ParametersType, DcaDataType, ScalarType>::symmetrizeLeadingEigenvectors() {
  profiler_type prof(__FUNCTION__, "BseLatticeSolver", __LINE__);

  if (concurrency.id() == concurrency.first())
    std::cout << "\n" << __FUNCTION__ << std::endl;

  for (int i = 0; i < num_evals; i++) {
    const int num_phases = 1000;
    std::complex<ScalarType> phase_min = 0;
    ScalarType diff_min = 1.e6;  // Initialize with some big number.

    // Find the phase that minizes \sum |g(\omega_)-g^*(-\omega_n)|, where the sum runs over the two
    // band indices, momentum, and half of the Matsubara frequencies.
    for (int l = 0; l < num_phases; l++) {
      const std::complex<ScalarType> phase(std::cos((2. * M_PI * l) / num_phases),
                                           std::sin((2. * M_PI * l) / num_phases));
      ScalarType diff = 0;

      for (int w = 0; w < w_VERTEX::dmn_size() / 2; w++)
        for (int k = 0; k < k_HOST_VERTEX::dmn_size(); k++)
          for (int b2 = 0; b2 < b::dmn_size(); b2++)
            for (int b1 = 0; b1 < b::dmn_size(); b1++)
              diff += std::abs(
                  phase * leading_eigenvectors(i, b1, b2, k, w) -
                  conj(phase * leading_eigenvectors(i, b1, b2, k, w_VERTEX::dmn_size() - 1 - w)));

      if (diff < diff_min) {
        diff_min = diff;
        phase_min = phase;
      }
    }

    if (diff_min > 1.e-3)
      std::logic_error("No phase found to symmetrize eigenvector.");

    for (int j = 0; j < LatticeEigenvectorDmn::dmn_size(); j++)
      leading_eigenvectors(i, j) *= phase_min;
  }
}

template <typename ParametersType, typename DcaDataType, typename ScalarType>
void BseLatticeSolver<ParametersType, DcaDataType, ScalarType>::characterize_leading_eigenvectors() {
  profiler_type prof(__FUNCTION__, "BseLatticeSolver", __LINE__);

  for (int n_lam = 0; n_lam < num_evals; n_lam++) {
    for (int w_ind = 0; w_ind < w_VERTEX::dmn_size(); w_ind++) {
      for (int m_ind = 0; m_ind < b::dmn_size(); m_ind++) {
        for (int n_ind = 0; n_ind < b::dmn_size(); n_ind++) {
          for (int n_cub = 0; n_cub < N_CUBIC; n_cub++) {
            std::complex<ScalarType> scal_prod = 0;

            std::complex<ScalarType> norm_phi = 0;
            std::complex<ScalarType> norm_psi = 0;

            for (int k_ind = 0; k_ind < k_HOST_VERTEX::dmn_size(); k_ind++) {
              std::complex<ScalarType> phi_k_val =
                  leading_eigenvectors(n_lam, m_ind, n_ind, k_ind, w_ind);
              std::complex<ScalarType> psi_k_val = leading_symmetry_functions(k_ind, n_cub);

              scal_prod += conj(psi_k_val) * phi_k_val;

              norm_phi += conj(phi_k_val) * phi_k_val;
              norm_psi += conj(psi_k_val) * psi_k_val;
            }

            leading_symmetry_decomposition(n_lam, m_ind, n_ind, n_cub, w_ind) =
                scal_prod / std::sqrt(1.e-16 + norm_phi * norm_psi);
          }
        }
      }
    }
  }
}

}  // analysis
}  // phys
}  // dca

#endif  // DCA_PHYS_DCA_ANALYSIS_BSE_SOLVER_BSE_LATTICE_SOLVER_HPP
