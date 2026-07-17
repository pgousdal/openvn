# Amiga Target Profiles and Package Format

M5 PR1 defines three initial targets:

- `amiga-ocs`
- `amiga-aga`
- `amiga-rtg`

## Source and runtime formats

OpenVN projects may use modern source assets such as PNG, WAV and OGG.

Classic profiles declare conversion policies:

- PNG to IFF ILBM
- WAV sound effects to IFF 8SVX
- music to ProTracker MOD

RTG profiles may use AmigaOS DataTypes and retain PNG, WAV or OGG when the
target system provides the required datatype modules.

## Runtime architecture

The runtime is:

- ARexx-first
- native C where required
- system-friendly
- based on direct AmigaOS libraries and devices
- independent of ACE

The native layer should use facilities such as `exec.library`,
`dos.library`, `intuition.library`, `graphics.library`, `datatypes.library`,
`audio.device`, `timer.device` and `input.device` where appropriate.

## Package layout

```text
manifest.json
story/
  story.openvn.json
  main.rexx
assets/
runtime/
  openvn-player
```
