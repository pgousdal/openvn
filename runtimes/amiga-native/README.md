# OpenVN Native Amiga Player

M5 PR3 introduces the native player skeleton.

## Architecture

- Public ARexx port: `OPENVNPLAYER`
- ARexx-first orchestration
- Native C command dispatcher
- Direct AmigaOS integration
- No ACE dependency

## Commands

- `LOAD <story>`
- `RUN`
- `SCENE <background>`
- `SHOW <character> <pose>`
- `HIDE <character>`
- `MUSIC <track-or-stop>`
- `SOUND <effect>`
- `QUIT`

## Host contract tests

```bash
make -C runtimes/amiga-native test
```

The host executable reads the same command protocol from standard input. This
lets CI test the protocol without emulating AmigaOS.

## AmigaOS implementation

`main_amiga.c` creates the `OPENVNPLAYER` message port and receives ARexx
messages through `rexxsyslib.library`.

The next milestone adds real story loading and AmigaOS graphics/audio service
adapters.
