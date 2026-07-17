# Vertical Slice Demo

`examples/demo` is the smallest complete OpenVN project that exercises the shared authoring and export pipeline.

Run:

```sh
openvn build examples/demo --clean
```

The command compiles `main.ink` once, then emits:

- `dist/renpy` — a runnable Ren'Py project.
- `dist/amiga-ocs` — an Amiga OCS package with generated C story/asset tables, ILBM graphics, MOD music, and 8SVX sound.
- `dist/build.json` — a machine-readable summary of the build outputs.

The demo deliberately includes a background, character pose, branching choice, two endings, MOD music, and a sound effect. It is intended as the end-to-end regression fixture for the OpenVN architecture.
