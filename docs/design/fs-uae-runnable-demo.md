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
