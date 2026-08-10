# M6 PR1h — Amiga SDK linker support

This note preserves the recovery instructions that temporarily replaced the
root README during native runtime bring-up.

PR1h addressed these m68k build diagnostics:

- `undefined reference to DrawDTObject`
- a missing prototype warning for `CreatePort`

The patch added `<clib/alib_protos.h>` to `main_amiga.c` and linked the Amiga
executable with the SDK's `amiga.lib` through CMake's `amiga` library name.

The historical recovery command was:

```sh
python3 scripts/apply-m6-pr1h.py
./scripts/build-m68k-demo-player.sh
```

The repository now contains the resulting linker configuration directly; the
patch script remains only as recovery history and is not part of the normal
build workflow.
