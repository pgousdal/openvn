# Diagnostics Integration

M3 PR5.2 connects structured diagnostics to the parser, story graph,
validator, compiler and CLI.

## Diagnostic codes

- `OVN001`: Ink syntax or source error
- `OVN002`: unresolved graph target
- `OVN003`: duplicate knot
- `OVN004`: unreachable node warning
- `OVN005`: invalid OpenVN story structure

## Text format

```text
main.ink:7:1: error OVN001: choice must use '* [Text] -> target'
```

Parser errors carry source file, line and column information. Graph
diagnostics currently identify semantic nodes and targets; source maps are
planned for a later milestone.

Warnings do not make validation fail. Errors do.
