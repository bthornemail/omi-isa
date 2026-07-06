# Mesh Protocol

The OMI mesh layer provides multi-hop routing over LoRa and other transports.

| Marker | Orientation[0] | Purpose |
|--------|----------------|---------|
| 0x00   | Data           | Normal data envelope |
| 0x01   | Route update   | DSDV-style route table flood |
| 0x02   | RREQ           | On-demand route request |
| 0x03   | RREP           | On-demand route reply |

See [omi_mesh.h](../lib/omi_mesh.h) and [omi_mesh.c](../lib/omi_mesh.c) for implementation.
