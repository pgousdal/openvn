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
  runtime/openvn-player
  story/
  assets/
package.json
README.txt
```

## Minimal bootstrap

A directory-backed hard drive may not contain an AmigaOS `C:` command
directory. The OpenVN package therefore occupies the volume root, where its
relative asset paths resolve, and `Startup-Sequence` invokes the player
directly:

```text
runtime/openvn-player
```

It must not require `Assign`, `Run`, `Wait`, `RX`, or other external commands.
Optional ARexx integration belongs inside the runtime and must not be part of
the bootstrap contract.

## Launch and manual validation

The target build and ROM requirements are documented in
[`../amiga-development.md`](../amiga-development.md). Once the package exists:

```sh
OPENVN_FS_UAE_KICKSTART=/path/to/amiga-os-3.x.rom \
OPENVN_FS_UAE_SYSTEM_DIR=/path/to/amiga-os-3.x-system \
./scripts/run-amiga-demo.sh
```

FS-UAE requires a legally obtained Kickstart ROM configured outside the
repository. OpenVN requires AmigaOS 3.x APIs; the launcher deliberately rejects
implicit ROM selection because the A500 model default may be Kickstart 1.3.
It makes a temporary writable copy of the explicitly supplied system directory,
boots that complete installation as DH0, and mounts the generated package as
DH1. The temporary startup launches OpenVN before Workbench; it is deleted when
FS-UAE exits. ROM and system files remain outside the generated package and
repository.
Package creation is automated; graphics, interactive branching and continuous
MOD playback remain a repeatable manual validation gate.
