# Envelope Specification

The 512-bit (64-byte) OMI envelope is the fundamental data unit.

| Offset | Size | Field       | Description |
|--------|------|-------------|-------------|
| 0      | 8    | gauge[8]    | Canonical pre-header (0xFF 00 1C 1D 1E 1F 20 FF) |
| 8      | 8    | orientation[8] | Packet type markers, P2P flags |
| 16     | 8    | recovery[8] | Originator node ID, sequence, TTL |
| 24     | 8    | target[8]   | Destination node ID, opcode (target[0] & 0x1F) |
| 32     | 32   | relation[32] | Payload data, audio samples, or routing entries |
