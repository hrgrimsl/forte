/*
 * @BEGIN LICENSE
 *
 * Forte: an open-source plugin to Psi4 (https://github.com/psi4/psi4)
 * that implements a variety of quantum chemistry methods for strongly
 * correlated electrons.
 *
 * Copyright (c) 2012-2023 by its authors (see COPYING, COPYING.LESSER, AUTHORS).
 *
 * The copyrights for code used from other parties are included in
 * the corresponding files.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this program.  If not, see http://www.gnu.org/licenses/.
 *
 * @END LICENSE
 */

#ifndef _fci_vector_
#define _fci_vector_

#include <vector>

#include "psi4/libmints/dimension.h"

#define CAPRICCIO_USE_DAXPY 1
#define CAPRICCIO_USE_UNROLL 0

namespace psi {
class Matrix;
class Vector;
} // namespace psi

namespace forte {
class ActiveSpaceIntegrals;
class BinaryGraph;
class MOSpaceInfo;
class StringLists;

class FCIVector {
  public:
    FCIVector(std::shared_ptr<StringLists> lists, size_t symmetry);
    ~FCIVector();

    //    // Simple operation
    void print();
    void zero();
    /// The size of the CI basis
    size_t size() const { return ndet_; }

    /// Copy the wave function object
    void copy(FCIVector& wfn);
    /// Copy the coefficient from a Vector object
    void copy(std::shared_ptr<psi::Vector> vec);
    /// Copy the wave function object
    void copy_to(std::shared_ptr<psi::Vector> vec);

    /// Form the diagonal part of the Hamiltonian
    void form_H_diagonal(std::shared_ptr<ActiveSpaceIntegrals> fci_ints);

    //    double approximate_spin(double )

    //    /// Initial guess
    //    void initial_guess(FCIVector& diag, size_t num_dets = 100);

    ////    void set_to(Determinant& det);
    void set(std::vector<std::tuple<size_t, size_t, size_t, double>>& sparse_vec);
    //    double get(int n);
    //    void plus_equal(double factor,FCIVector& wfn);
    //    void scale(double factor);
    double norm(double power = 2.0);
    ////    void normalize_wrt(Determinant& det);
    void normalize();
    double dot(FCIVector& wfn);
    double dot(std::shared_ptr<FCIVector>& wfn);

    std::vector<double>& opdm_a() { return opdm_a_; }
    std::vector<double>& opdm_b() { return opdm_b_; }
    std::vector<double>& tpdm_aa() { return tpdm_aa_; }
    std::vector<double>& tpdm_ab() { return tpdm_ab_; }
    std::vector<double>& tpdm_bb() { return tpdm_bb_; }
    std::vector<double>& tpdm_aaa() { return tpdm_aaa_; }
    std::vector<double>& tpdm_aab() { return tpdm_aab_; }
    std::vector<double>& tpdm_abb() { return tpdm_abb_; }
    std::vector<double>& tpdm_bbb() { return tpdm_bbb_; }

    // Operations on the wave function
    void Hamiltonian(FCIVector& result, std::shared_ptr<ActiveSpaceIntegrals> fci_ints);

    double energy_from_rdms(std::shared_ptr<ActiveSpaceIntegrals> fci_ints);

    void compute_rdms(int max_order = 2);
    void rdm_test();

    /// Compute the expectation value of the S^2 operator
    double compute_spin2();

    /// Print the natural_orbitals from FCIWFN
    /// Assume user specified active space
    void print_natural_orbitals(std::shared_ptr<MOSpaceInfo>);

    /// Return the elements with the smallest value
    /// This function returns the tuple (C_I,irrep,Ia,Ib)
    std::vector<std::tuple<double, size_t, size_t, size_t>> min_elements(size_t num_dets);
    /// Return the elements with the largest absolute value
    /// This function returns the tuple (|C_I|,C_I,irrep,Ia,Ib)
    std::vector<std::tuple<double, double, size_t, size_t, size_t>>
    max_abs_elements(size_t num_dets);

    // Temporary memory allocation
    static void allocate_temp_space(std::shared_ptr<StringLists> lists_, int print_);
    static void release_temp_space();
    void set_print(int print) { print_ = print; }

  private:
    // ==> Class Data <==

    /// The number of irreps
    int nirrep_;
    /// The symmetry of this vector
    size_t symmetry_;
    /// The total number of correlated molecular orbitals
    size_t ncmo_;
    /// The number of correlated molecular orbitals per irrep
    psi::Dimension cmopi_;
    /// The offset array for cmopi_
    std::vector<size_t> cmopi_offset_;
    /// The number of determinants
    size_t ndet_;
    /// The number of determinants per irrep
    std::vector<size_t> detpi_;
    /// The print level
    int print_ = 0;

    /// The string list
    std::shared_ptr<StringLists> lists_;
    // Graphs
    /// The alpha string graph
    std::shared_ptr<BinaryGraph> alfa_graph_;
    /// The beta string graph
    std::shared_ptr<BinaryGraph> beta_graph_;
    /// Coefficient matrix stored in block-matrix form
    std::vector<std::shared_ptr<psi::Matrix>> C_;
    std::vector<double> opdm_a_;
    std::vector<double> opdm_b_;
    std::vector<double> tpdm_aa_;
    std::vector<double> tpdm_ab_;
    std::vector<double> tpdm_bb_;
    std::vector<double> tpdm_aaa_;
    std::vector<double> tpdm_aab_;
    std::vector<double> tpdm_abb_;
    std::vector<double> tpdm_bbb_;

    // ==> Class Static Data <==

    static std::shared_ptr<psi::Matrix> C1;
    static std::shared_ptr<psi::Matrix> Y1;
    static size_t sizeC1;

    // Timers
    static double hdiag_timer;
    static double h1_aa_timer;
    static double h1_bb_timer;
    static double h2_aaaa_timer;
    static double h2_aabb_timer;
    static double h2_bbbb_timer;

    // ==> Class Public Functions <==

    void startup();
    void cleanup();

    // ==> Class Private Functions <==

    size_t oei_index(size_t p, size_t q) const { return ncmo_ * p + q; }
    size_t tei_index(size_t p, size_t q, size_t r, size_t s) const {
        return ncmo_ * ncmo_ * ncmo_ * p + ncmo_ * ncmo_ * q + ncmo_ * r + s;
    }
    size_t six_index(size_t p, size_t q, size_t r, size_t s, size_t t, size_t u) const {
        return (ncmo_ * ncmo_ * ncmo_ * ncmo_ * ncmo_ * p + ncmo_ * ncmo_ * ncmo_ * ncmo_ * q +
                ncmo_ * ncmo_ * ncmo_ * r + ncmo_ * ncmo_ * s + ncmo_ * t + u);
    }

    void H0(FCIVector& result, std::shared_ptr<ActiveSpaceIntegrals> fci_ints);
    void H1(FCIVector& result, std::shared_ptr<ActiveSpaceIntegrals> fci_ints, bool alfa);
    void H2_aabb(FCIVector& result, std::shared_ptr<ActiveSpaceIntegrals> fci_ints);
    void H2_aaaa2(FCIVector& result, std::shared_ptr<ActiveSpaceIntegrals> fci_ints, bool alfa);

    // 1-RDM elements are stored in the format
    // <a^+_{pa} a^+_{qb} a_{sb} a_ra> -> rdm[oei_index(p,q)]

    /// Compute the matrix elements of the same 1-RDM <a^+_{p} a_{q}>
    void compute_1rdm(std::vector<double>& rdm, bool alfa);

    // 2-RDM elements are stored in the format
    // <a^+_{p} a^+_{q} a_{s} a_r> -> rdm[tei_index(p,q,r,s)]

    /// Compute the matrix elements of the same spin 2-RDM <a^+_p a^+_q a_s a_r> (with all indices
    /// alpha or beta)
    void compute_2rdm_aa(std::vector<double>& rdm, bool alfa);
    /// Compute the matrix elements of the alpha-beta 2-RDM <a^+_{pa} a^+_{qb} a_{sb} a_{ra}>
    void compute_2rdm_ab(std::vector<double>& rdm);

    // 3-RDM elements are stored in the format
    // <a^+_p a^+_q a^+_r a_u a_t a_s> -> rdm[six_index(p,q,r,s,t,u)]

    /// Compute the matrix elements of the same spin 3-RDM <a^+_p a^+_q a_s a_r> (with all indices
    /// alpha or beta)
    void compute_3rdm_aaa(std::vector<double>& rdm, bool alfa);
    /// Compute the matrix elements of the alpha-alpha-beta 3-RDM <a^+_{pa} a^+_{qa} a^+_{rb} a_{ub}
    /// a_{ta} a_{sa}>
    void compute_3rdm_aab(std::vector<double>& rdm);
    /// Compute the matrix elements of the alpha-beta-beta 3-RDM <a^+_{pa} a^+_{qb} a^+_{rb} a_{ub}
    /// a_{tb} a_{sa}>
    void compute_3rdm_abb(std::vector<double>& rdm);
};
} // namespace forte

#endif // _fci_vector_

////    DetAddress get_det_address(Determinant& det) {
////        int sym = alfa_graph_->sym(det.get_alfa_bits());
////        size_t alfa_string = alfa_graph_->rel_add(det.get_alfa_bits());
////        size_t beta_string = beta_graph_->rel_add(det.get_beta_bits());
////        return DetAddress(sym,alfa_string,beta_string);
////    };
