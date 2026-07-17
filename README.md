# M6 PR1h — Amiga SDK linker support

Fixes the final m68k link errors:

- `undefined reference to DrawDTObject`
- missing prototype warning for `CreatePort`

Changes:

1. Adds `<clib/alib_protos.h>` to `main_amiga.c`.
2. Links the Amiga executable with `amiga.lib` through CMake's `amiga`
   library name.

Apply:

```sh
python3 scripts/apply-m6-pr1h.py
./scripts/build-m68k-demo-player.sh
```
