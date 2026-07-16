# JSON Diagnostics and Strict Validation

M3 PR5.3 completes the first diagnostics milestone.

## JSON validation output

```bash
openvn validate examples/minimal --json
```

Output:

```json
{
  "ok": true,
  "diagnostics": []
}
```

Diagnostics use the same fields as the internal `Diagnostic` model.

## Strict compilation

```bash
openvn compile examples/minimal --strict
```

Strict mode treats warnings as blocking. In particular, unreachable nodes
reported as `OVN004` stop compilation.

## Schema validation

The compiler validates its generated semantic document against
`schemas/openvn-story-0.3.schema.json` before writing the output file.
Schema failures use diagnostic code `OVN005`.
