#ifndef _orbital_transform_h_
#define _orbital_transform_h_

#include "base_classes/scf_info.h"
#include "base_classes/state_info.h"
#include "base_classes/forte_options.h"
#include "integrals/integrals.h"

namespace forte {

class OrbitalTransform {

  public:
    /**

    **/
    OrbitalTransform(std::shared_ptr<ForteIntegrals> ints,
                     std::shared_ptr<MOSpaceInfo> mo_space_info);

    /// Default constructor
    OrbitalTransform() = default;

    /// Virtual destructor to enable deletion of a Derived* through a Base*
    virtual ~OrbitalTransform() = default;

    virtual void compute_transformation() = 0;

    psi::SharedMatrix get_Ua() { return Ua_; };

    psi::SharedMatrix get_Ub() { return Ub_; };

  protected:
    // The integrals
    std::shared_ptr<ForteIntegrals> ints_;
    /// The MOSpace info
    std::shared_ptr<MOSpaceInfo> mo_space_info_;

    /// @brief Unitary matrix for alpha orbital rotations
    psi::SharedMatrix Ua_;
    /// @brief Unitary matrix for beta orbital rotations
    psi::SharedMatrix Ub_;
};

std::unique_ptr<OrbitalTransform>
make_orbital_transformation(const std::string& type, std::shared_ptr<SCFInfo> scf_info,
                            std::shared_ptr<ForteOptions> options,
                            std::shared_ptr<ForteIntegrals> ints,
                            std::shared_ptr<MOSpaceInfo> mo_space_info);

} // namespace forte

#endif
