#!/usr/bin/env python
# -*- coding: utf-8 -*-


def test_uccsd():
    """Test projective UCCSD on H2 using RHF/DZ orbitals"""

    import pytest
    import forte.proc.scc as scc
    import forte
    import psi4

    ref_energy = -1.126712715716011  # UCCSD = FCI energy from psi4

    geom = """
     H
     H 1 1.0
    """

    scf_energy, psi4_wfn = forte.utils.psi4_scf(geom, basis='DZ', reference='RHF')
    forte_objs = forte.utils.prepare_forte_objects(psi4_wfn, mo_spaces={})
    calc_data = scc.run_cc(
        forte_objs['as_ints'],
        forte_objs['scf_info'],
        forte_objs['mo_space_info'],
        cc_type='ucc',
        max_exc=2,
        e_convergence=1.0e-11
    )

    psi4.core.clean()

    energy = calc_data[-1][1]

    print(f'  HF energy:    {scf_energy}')
    print(f'  UCCSD energy: {energy}')
    print(f'  E - Eref:     {energy - ref_energy}')

    assert energy == pytest.approx(ref_energy, 1.0e-11)


if __name__ == "__main__":
    test_uccsd()
