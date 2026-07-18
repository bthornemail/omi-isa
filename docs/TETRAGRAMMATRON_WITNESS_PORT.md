# Tetragrammatron Witness OMI-Lisp to OMI-ISA Port

This fixture is the second controlled executable law port after the NULL Ring fixture. It is not a bulk import of the `omi-tetragrammatron` repository.

`programs/tetragrammatron_witness.omi` is the readable OMI source fixture. `test/test_tetragrammatron_witness.c` is the bounded OMI-ISA executable witness for the same law.

The fixture establishes the Tetragrammatron as the Polyharmonic Governor and
Polybius gauge governor.

The primary bounded axis is:

```text
3 clocks
  Atomic Logic Clock
  Spectral Observer Clock
  Cosmic Orbit Clock

4 visible offsets
  FS = 0x0001
  GS = 0x0010
  RS = 0x0100
  US = 0x1000

5 governor modes
  FACTS        p=-1
  RULES        p=0
  CLOSURES     p=1
  COMBINATORS  p=2
  CONS         p=3
```

The offsets advance by one nibble. The governor exponents are consecutive,
with RULES at the `p=0` Genesis equality pivot and FACTS/CONS as the declared
circular endpoints.

The Miquel incidence controller is:

```text
8 points
6 blocks
3 blocks through each point
4 points in each block
8 x 3 = 6 x 4 = 24 flags
```

The OMI oversight count is:

```text
5 hidden governor positions
+ 6 pairwise gauge planes
= 11 oversight positions
```

This is an OMI eleven-position frame. It is not a claim of identity with the
standard abstract regular 11-cell.

The local hexadecimal gauge is:

```text
0 1 2 3
4 5 6 7
8 9 A B
C D E F
```

The tetrahedral diagonals are:

```text
D+ = {0,5,A,F}
D- = {3,6,9,C}
```

The diagonal closures are:

```text
0 + 5 + A + F = 0x1E
3 + 6 + 9 + C = 0x1E
```

So `0x1E` is both the diagonal closure witness and the ASCII Record Separator.

The full nibble field closes by sum:

```text
0 + 1 + 2 + ... + F = 0x78
```

So `0x78` is the full hexadecimal system witness.

Inside that governor frame, the fold-carry pair is:

```text
0x3C = centered fold witness
0x78 = shifted / carry fold witness
0x78 == (0x3C << 1) & 0xFF
0x3C == (0x78 >> 1)
0x3C has 4 one bits and 4 zero bits
0x78 has 4 one bits and 4 zero bits
0x3C ^ 0x78 = 0x44
```

Tetragrammatron governs lawful Polybius gauge transitions and validates fold/carry. It does not project surfaces and does not scribe crossings. Metatron owns the `0xAA55 / 0x55AA` balanced alternating witness in a separate fixture.

The corresponding finite Coq proof is
`../../omi-axioms/coq/PolyharmonicGovernor.v`. It proves the bounded axis
counts, nibble offset stepping, governor exponent sequence, explicit Miquel
degrees and flags, OMI `5+6=11` oversight count, and the distinction between
validation and acceptance.

The fixture does not add compiler keywords, validate arbitrary frames, accept
receipts, or alter lowering. The executable surface uses fixed-width integer
operations only. There are no filesystem, network, time, random, or external
runtime dependencies.
