# Tetragrammatron Witness OMI-Lisp to OMI-ISA Port

This fixture is the second controlled executable law port after the NULL Ring
fixture. It is not a bulk import of the `omi-tetragrammatron` repository.

`programs/tetragrammatron_witness.omi` is the readable OMI source fixture.
`test/test_tetragrammatron_witness.c` is the bounded OMI-ISA executable witness
for the same law.

The fixture establishes the alternating witness pair:

```text
0xAA55 has 8 one bits and 8 zero bits
0x55AA has 8 one bits and 8 zero bits
0xAA55 ^ 0x55AA = 0xFFFF
0xAA55 ^ 0xFFFF = 0x55AA
0x55AA ^ 0xFFFF = 0xAA55
```

The tetragrammatron witness is observation and projection only. It does not
validate state and does not accept state. Validation authority remains outside
this fixture until a later, explicitly bounded port promotes that behavior.

The executable surface uses fixed-width integer operations only. There are no
filesystem, network, time, random, or external repository dependencies.
