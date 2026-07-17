# Generated Amiga Story Runtime

M5 PR4.1 replaces runtime JSON parsing with compiler-generated C tables.

## Pipeline

```text
story.openvn.json
        ↓
Amiga exporter
        ↓
story.generated.h
story.generated.c
        ↓
native OpenVN player
```

The JSON file remains in the package for debugging and tooling, but the native
runtime does not parse it.

## Benefits

- no JSON parser in the Amiga executable
- no heap allocation for the story graph
- deterministic static data
- faster startup
- smaller and more robust runtime
- easier cross-compilation and debugging

The generated tables contain immutable node and choice data. Runtime state
contains only the attached story pointer, current node index, and execution
flags.

This preserves the ARexx-first and system-friendly native AmigaOS design. The
native C layer remains independent of ACE.
