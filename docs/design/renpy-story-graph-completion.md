# Ren'Py Story Graph Completion

M4 PR2 completes Story Format 0.3 handling for the Ren'Py backend.

Features:

- deterministic output
- stable and validated labels
- validation of entry and every graph target
- Unicode-safe text emission
- empty text support
- complete text, choice, jump and end handling
- golden output tests
- `--clean` export support

Example:

```bash
openvn export examples/renpy-branching   --backend renpy   --output build/renpy-branching   --clean
```
