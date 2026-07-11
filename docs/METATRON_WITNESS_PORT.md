# Metatron Witness OMI-Lisp to OMI-ISA Port

This fixture is the third controlled executable law port after the NULL Ring
and Tetragrammatron witness fixtures. It is not a bulk import of an
`omi-metatron` repository.

`programs/metatron_witness.omi` is the readable OMI source fixture.
`test/test_metatron_witness.c` is the bounded OMI-ISA executable witness for the
same law.

The fixture establishes the Metatron alternating witness pair:

```text
0xAA55 has 8 one bits and 8 zero bits
0x55AA has 8 one bits and 8 zero bits
0xAA55 ^ 0x55AA = 0xFFFF
0xAA55 ^ 0xFFFF = 0x55AA
0x55AA ^ 0xFFFF = 0xAA55
```

Metatron is not validation authority in this fixture. It witnesses and projects
only after validation has already happened elsewhere. This keeps the executable
surface small and prevents projection from becoming acceptance.

The executable surface uses fixed-width integer operations only. There are no
filesystem, network, time, random, or external repository dependencies.
