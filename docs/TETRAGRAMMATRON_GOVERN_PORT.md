# Tetragrammatron Govern OMI-Lisp to OMI-ISA Port

This fixture makes the Polybius gauge and fold-carry constants executable in the
bounded OMI-ISA test surface. It complements the Tetragrammatron witness fixture;
it does not import the full `omi-tetragrammatron` repository.

`programs/tetragrammatron_govern.omi` is the readable OMI source fixture.
`test/test_tetragrammatron_govern.c` is the bounded executable witness for the
same law.

The local hexadecimal gauge is:

```text
0 1 2 3
4 5 6 7
8 9 A B
C D E F
```

The tetrahedral diagonals close to the same record-separator witness:

```text
D+ = 0 + 5 + A + F = 0x1E
D- = 3 + 6 + 9 + C = 0x1E
```

The full nibble field closes to the system witness:

```text
0 + 1 + 2 + ... + F = 0x78
```

The fold-carry relation from the kernel reader declaration opener is:

```text
0x3C << 1 = 0x78
```

The byte gauge remains `0x00..0x7F`. `0xAA55` is a 16-bit external bridge word
and enters through a bridge resolver, not through the byte gauge.

The executable surface uses fixed-width integer operations only. There are no
filesystem, network, time, random, or external repository dependencies.
