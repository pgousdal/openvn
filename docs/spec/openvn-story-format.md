# OpenVN Story Format 0.3

OpenVN Story Format 0.3 is the stable intermediate representation used by
OpenVN compiler and runtime backends.

## Top-level fields

- `format`: always `openvn-story`
- `version`: always `0.3`
- `entry`: first node executed by the runtime
- `symbols`: source-level knot names mapped to node IDs
- `nodes`: ordered semantic node collection

## Node types

### Text node

Fields:

- `id`
- `type`: `text`
- `text`
- `next`: next node ID or `null`

### Choice node

Fields:

- `id`
- `type`: `choice`
- `options`

Each option contains display text and a target node ID.

### Jump node

Fields:

- `id`
- `type`: `jump`
- `target`

### End node

Fields:

- `id`
- `type`: `end`

## Compatibility

Backends targeting format 0.3 must reject unsupported future major or minor
format versions instead of silently interpreting them.
