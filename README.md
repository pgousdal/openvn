# OpenVN

OpenVN is an open, runtime-independent visual novel toolchain.

## Ren'Py asset export

```bash
uv run --project compiler openvn export examples/demo   --backend renpy   --output build/renpy-demo   --clean
```

The generated project includes script, image and audio declarations plus
copied assets.

The Amiga runtime remains ARexx-first and native-C-based, without ACE.
