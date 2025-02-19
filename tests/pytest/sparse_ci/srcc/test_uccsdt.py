#!/usr/bin/env python
# -*- coding: utf-8 -*-

import pytest


@pytest.mark.skip(reason="This is a long test")
def test_uccsdt():
    """Test projective UCCSDT on Ne using RHF/cc-pVDZ orbitals"""

    import forte.proc.scc as scc
    import forte
    import psi4

    ref_energy = -128.679016412  # from Evangelista, J. Chem. Phys. 134, 224102 (2011).

    geom = "Ne"

    scf_energy, psi4_wfn = forte.utils.psi4_scf(geom, basis='cc-pVDZ', reference='RHF')
    forte_objs = forte.utils.prepare_forte_objects(psi4_wfn, mo_spaces={'FROZEN_DOCC': [1, 0, 0, 0, 0, 0, 0, 0]})
    calc_data = scc.run_cc(
        forte_objs['as_ints'],
        forte_objs['scf_info'],
        forte_objs['mo_space_info'],
        cc_type='ucc',
        max_exc=3,
        e_convergence=1.0e-10
    )

    psi4.core.clean()

    energy = calc_data[-1][1]

    print(f'  HF energy:     {scf_energy}')
    print(f'  UCCSDT energy: {energy}')
    print(f'  E - Eref:      {energy - ref_energy}')

    assert energy == pytest.approx(ref_energy, 1.0e-11)


if __name__ == "__main__":
    test_uccsdt()
