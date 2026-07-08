# NULL Ring OMI-Lisp to OMI-ISA Port

This fixture is a small controlled executable law port, not a bulk inventory
conversion.

`programs/null_ring.omi` is the readable OMI source fixture. `test/test_null_ring.c`
is the bounded OMI-ISA executable witness for the same law.

The NULL Ring law fixes the dot relation as XOR over a 16-bit bounded execution
surface:

```text
(NULL . NULL) -> 0x0000
0x00 ^ 0x20 -> 0x20
0x20 ^ 0x7F -> 0x5F
0x7F ^ 0xFF -> 0x80
0xFF ^ 0x00 -> 0xFF
0x20 ^ 0x5F ^ 0x80 ^ 0xFF -> 0x00
```

There are no filesystem, network, time, or random dependencies. OMI-ISA is the
bounded executable surface for this fixture. Future cross-repo ports can use the
same pattern: keep the OMI declaration readable, keep the executable witness
small, and preserve a receipt/test trace before adding broader behavior.
