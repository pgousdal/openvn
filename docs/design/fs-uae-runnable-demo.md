# Runnable FS-UAE demo package

The `openvn package-fsuae` command owns the complete generated FS-UAE tree.
Consumers must not manually edit files below `dist/fs-uae`, because `--clean`
recreates that directory from the Amiga export package and player executable.

## Generated pipeline

```text
OpenVN project
    -> openvn build
    -> dist/amiga-ocs
    -> m68k openvn-player
    -> openvn package-fsuae --clean
    -> dist/fs-uae
```

The generated directory contains:

```text
OpenVNDemo.fs-uae
harddrive/
  S/Startup-Sequence
  OpenVN/
    runtime/openvn-player
    story/
    assets/
package.json
README.txt
```

## Minimal bootstrap

A directory-backed hard drive may not contain an AmigaOS `C:` command
directory. The generated `Startup-Sequence` therefore invokes the player by
absolute Amiga path:

```text
DH0:OpenVN/runtime/openvn-player
```

It must not require `Assign`, `Run`, `Wait`, `RX`, or other external commands.
Optional ARexx integration belongs inside the runtime and must not be part of
the bootstrap contract.

## Launch and manual validation

The target build and ROM requirements are documented in
[`../amiga-development.md`](../amiga-development.md). Once the package exists:

```sh
fs-uae examples/demo/dist/fs-uae/OpenVNDemo.fs-uae
```

FS-UAE requires a legally obtained Kickstart ROM configured outside the
repository. Package creation is automated; graphics, interactive branching and
continuous MOD playback are currently a repeatable manual validation gate.
