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

#ifndef _active_space_solver_h_
#define _active_space_solver_h_

#include <map>
#include <vector>
#include <string>

#include "psi4/libmints/matrix.h"

#include "base_classes/state_info.h"
#include "base_classes/rdms.h"
#include "integrals/one_body_integrals.h"
#include "sparse_ci/determinant_hashvector.h"
namespace forte {

class ActiveSpaceMethod;
class ActiveSpaceIntegrals;
class ForteIntegrals;
class ForteOptions;
class MOSpaceInfo;
class SCFInfo;

/**
 * @class ActiveSpaceSolver
 *
 * @brief General class for a multi-state active space solver
 *
 * This class can run state-specific, multi-state, and state-averaged computations
 * on small subset of the full orbital space (<30-40 orbitals).
 */
class ActiveSpaceSolver {
  public:
    // ==> Class Constructor and Destructor <==
    /**
     * @brief ActiveSpaceMethod Constructor for a multi-state computation
     * @param method A string that labels the method requested (e.g. "FCI", "ACI", ...)
     * @param nroots_map A map of electronic states to the number of roots computed {state_1 : n_1,
     * state_2 : n_2, ...} where state_i specifies the symmetry of a state and n_i is the number of
     * levels computed.
     * @param state information about the electronic state
     * @param mo_space_info a MOSpaceInfo object
     * @param as_ints integrals for active space
     */
    ActiveSpaceSolver(const std::string& method,
                      const std::map<StateInfo, size_t>& state_nroots_map,
                      std::shared_ptr<SCFInfo> scf_info, std::shared_ptr<MOSpaceInfo> mo_space_info,
                      std::shared_ptr<ActiveSpaceIntegrals> as_ints,
                      std::shared_ptr<ForteOptions> options);

    // ==> Class Interface <==

    /// Set the print level
    /// @param level the print level (0 = no printing, 1 default)
    void set_print(int level);

    /// Compute the energy and return it // TODO: document (Francesco)
    const std::map<StateInfo, std::vector<double>>& compute_energy();

    /// Compute permanent dipole moments
    void compute_dipole_moment(std::shared_ptr<ActiveMultipoleIntegrals> ampints);

    /// Compute permanent quadrupole moments
    void compute_quadrupole_moment(std::shared_ptr<ActiveMultipoleIntegrals> ampints);

    /// Compute transition dipole moments
    void compute_transition_dipole(std::shared_ptr<ActiveMultipoleIntegrals> ampints);

    /// Compute the oscillator strengths assuming same orbitals
    void compute_fosc_same_orbs(std::shared_ptr<ActiveMultipoleIntegrals> ampints);

    /// Compute the contracted CI energy
    const std::map<StateInfo, std::vector<double>>&
    compute_contracted_energy(std::shared_ptr<forte::ActiveSpaceIntegrals> as_ints,
                              int max_rdm_level);

    /// Compute RDMs of all states in the given map
    /// First entry of the pair corresponds to bra and the second is the ket.
    std::vector<std::shared_ptr<RDMs>> rdms(
        std::map<std::pair<StateInfo, StateInfo>, std::vector<std::pair<size_t, size_t>>>& elements,
        int max_rdm_level, RDMsType rdm_type);

    /// Compute the state-averaged reference
    std::shared_ptr<RDMs>
    compute_average_rdms(const std::map<StateInfo, std::vector<double>>& state_weights_map,
                         int max_rdm_level, RDMsType rdm_type);

    /// Compute the overlap of two wave functions acted by complementary operators
    /// Return a map from state to roots of values
    /// Computes the overlap <Ψ(N-1)|Ψ'(N-1)>, where the (N-1)-electron wave function is given by
    /// Ψ(N-1) = h_{pσ} (t) |Ψ (N)> = \sum_{uvw} t^{uv}_{pw} \sum_{σ1} w^+_{σ1} v_{σ1} u_{σ} |Ψ(N)>.
    /// Useful to get the 3-RDM contribution of fully contracted term of two 2-body operators:
    /// \sum_{puvwxyzστθ} v_{pwxy} t_{uvpz} <Ψ(N)| xσ^+ yτ^+ wτ zθ^+ vθ uσ |Ψ(N)>
    std::map<StateInfo, std::vector<double>>
    compute_complementary_H2caa_overlap(ambit::Tensor Tbra, ambit::Tensor Tket);

    /// Print a summary of the computation information
    void print_options();

    /// Return a map of StateInfo to the computed nroots of energies
    const std::map<StateInfo, std::vector<double>>& state_energies_map() const {
        return state_energies_map_;
    }

    /// Return a map of StateInfo to the CI wave functions (deterministic determinant space)
    std::map<StateInfo, psi::SharedMatrix> state_ci_wfn_map() const;

    /// Pass a set of ActiveSpaceIntegrals to the solver (e.g. an effective Hamiltonian)
    /// @param as_ints the pointer to a set of acitve-space integrals
    void set_active_space_integrals(std::shared_ptr<ActiveSpaceIntegrals> as_ints) {
        as_ints_ = as_ints;
    }

    /// Pass multipole integrals to the solver (e.g. correlation dressed dipole/quadrupole)
    void set_active_multipole_integrals(std::shared_ptr<ActiveMultipoleIntegrals> as_mp_ints) {
        as_mp_ints_ = as_mp_ints;
    }

    /// Return the map of StateInfo to the wave function file name
    std::map<StateInfo, std::string> state_filename_map() const { return state_filename_map_; }

    /// Save the wave function to disk
    void dump_wave_function();

    /// Set energy convergence
    void set_e_convergence(double e_convergence) { e_convergence_ = e_convergence; }

    /// Set residual convergence
    void set_r_convergence(double r_convergence) { r_convergence_ = r_convergence; }

    /// Set if read wave function from file as initial guess
    void set_read_initial_guess(bool read_guess) { read_initial_guess_ = read_guess; }

    /// Set unitary matrices for changing orbital basis in RDMs when computing dipole moments
    void set_Uactv(ambit::Tensor& Ua, ambit::Tensor& Ub) {
        Ua_actv_ = Ua;
        Ub_actv_ = Ub;
    }

  protected:
    /// a string that specifies the method used (e.g. "FCI", "ACI", ...)
    std::string method_;

    /// A map of electronic states to the number of roots computed
    ///   {state_1 : n_1, state_2 : n_2, ...}
    /// where state_i specifies the symmetry of a state and n_i is the number of levels computed.
    std::map<StateInfo, size_t> state_nroots_map_;

    /// The information about a previous SCF computation
    std::shared_ptr<SCFInfo> scf_info_;

    /// The MOSpaceInfo object
    std::shared_ptr<MOSpaceInfo> mo_space_info_;

    /// The molecular integrals for the active space
    /// This object holds only the integrals for the orbital contained in the
    /// active_mo_vector.
    /// The one-electron integrals and scalar energy contains contributions from the
    /// doubly occupied orbitals specified by the core_mo_ vector.
    std::shared_ptr<ActiveSpaceIntegrals> as_ints_;

    /// The multipole integrals for the active space
    std::shared_ptr<ActiveMultipoleIntegrals> as_mp_ints_;

    /// User-provided options
    std::shared_ptr<ForteOptions> options_;

    /// A map of state symmetries to the associated ActiveSpaceMethod
    std::map<StateInfo, std::shared_ptr<ActiveSpaceMethod>> state_method_map_;

    /// Make sure that the values of <S^2> are consistent with the multiplicity
    void validate_spin(const std::vector<double>& spin2, const StateInfo& state);

    /// Prints a summary of the energies with State info
    void print_energies();

    /// A map of state symmetries to vectors of computed energies under given state symmetry
    std::map<StateInfo, std::vector<double>> state_energies_map_;

    /// A map of state symmetries to vectors of computed average S^2 under given state symmetry
    std::map<StateInfo, std::vector<double>> state_spin2_map_;

    /// A map of state symmetries to the file name of wave function stored on disk
    std::map<StateInfo, std::string> state_filename_map_;

    /// A variable to control printing information
    int print_ = 1;

    /// The energy convergence criterion
    double e_convergence_ = 1.0e-10;

    /// The residual 2-norm convergence criterion
    double r_convergence_ = 1.0e-6;

    /// Read wave function from disk as initial guess
    bool read_initial_guess_;

    /// Only print the transitions between states with different gas
    bool gas_diff_only_;

    /// Unitary matrices for orbital rotations used to compute dipole moments
    /// The issue is dipole integrals are transformed to semi-canonical orbital basis,
    /// while active-space integrals are in the original orbital basis
    ambit::Tensor Ua_actv_;
    ambit::Tensor Ub_actv_;

    /// Pairs of state info and the contracted CI eigen vectors
    std::map<StateInfo, std::shared_ptr<psi::Matrix>>
        state_contracted_evecs_map_; // TODO move outside?
};                                   // namespace forte

/**
 * @brief Make an active space solver object.
 * @param type a string that specifies the type (e.g. "FCI", "ACI", ...)
 * @param state_nroots_map a map from state symmetry to the number of roots
 * @param scf_info information about a previous SCF computation
 * @param mo_space_info orbital space information
 * @param ints an integral object
 * @param options user-provided options
 * @return a shared pointer for the base class ActiveSpaceMethod
 */
std::shared_ptr<ActiveSpaceSolver> make_active_space_solver(
    const std::string& method, const std::map<StateInfo, size_t>& state_nroots_map,
    std::shared_ptr<SCFInfo> scf_info, std::shared_ptr<MOSpaceInfo> mo_space_info,
    std::shared_ptr<ActiveSpaceIntegrals> as_ints, std::shared_ptr<ForteOptions> options);

/**
 * @brief Convert a map of StateInfo to weight lists to a map of StateInfo to number of roots.
 * @param state_weights_map A map of StateInfo to weight lists
 * @return A map of StateInfo to number of states
 */
std::map<StateInfo, size_t>
to_state_nroots_map(const std::map<StateInfo, std::vector<double>>& state_weights_map);

/**
 * @brief Make a list of states and weights.
 * @param options user-provided options
 * @param mo_space_info orbital space information
 * @return a unique pointer to an ActiveSpaceSolver object
 */
std::map<StateInfo, std::vector<double>>
make_state_weights_map(std::shared_ptr<ForteOptions> options,
                       std::shared_ptr<forte::MOSpaceInfo> mo_space_info);

/**
 * @brief Compute the average energy for a set of states
 * @param state_energies_list a map of state -> energies
 * @param state_weight_list a map of state -> weights
 */
double
compute_average_state_energy(const std::map<StateInfo, std::vector<double>>& state_energies_map,
                             const std::map<StateInfo, std::vector<double>>& state_weight_map);

} // namespace forte

#endif // _active_space_solver_h_
