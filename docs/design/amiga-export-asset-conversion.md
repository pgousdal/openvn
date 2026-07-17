# Amiga Export and Asset Conversion

M5 PR2 adds the Amiga export command:

```bash
openvn export examples/the-last-lighthouse   --backend amiga   --profile amiga-rtg   --output build/lighthouse-amiga   --clean
```

Implemented conversions:

- PNG to IFF ILBM for OCS and AGA
- mono 16-bit WAV to IFF 8SVX for sound effects
- PNG/WAV/OGG passthrough for RTG profiles
- MOD passthrough for classic music

Classic Amiga music must currently be supplied as MOD. A mixed WAV or OGG
music file is not automatically converted to tracker music.

The generated package contains story JSON, an ARexx bootstrap, converted
assets and a runtime contract. The native player remains ARexx-first,
system-friendly, independent of ACE and based on direct AmigaOS services.
