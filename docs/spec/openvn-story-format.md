# OpenVN Story Format 0.4

OpenVN Story Format 0.4 is the stable intermediate representation used by
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

### Runtime variable nodes

`set_bool`, `set_int`, and `set_string` nodes contain `id`, `type`, `name`,
`value`, and nullable `next` fields. Integer values are restricted to signed
32-bit range. These nodes overwrite the named runtime variable without changing
the fixed store capacity.

### Condition node

A `condition` node contains:

- `id` and `type` (`condition`);
- a typed `condition` object with `variable_name`, `value_type`, `operator`, and
  literal value fields;
- `true_target` and `false_target`, both resolved node IDs.

Valid operators are `bool_true`, `bool_false`, `==`, `!=`, `<`, `<=`, `>`, and
`>=`, subject to the type restrictions documented in the runtime API. Backends
must not coerce variable types. An unknown variable or mismatch is a runtime
error rather than a false condition.

The Ink compiler represents blocks with the existing command convention:

```ink
#openvn if score >= 10
High score route.
#openvn else
Normal route.
#openvn end
```

It resolves both branches at compile time and emits a jump around the else body.

## Compatibility

Backends targeting format 0.4 must reject unsupported future major or minor
format versions instead of silently interpreting them.
