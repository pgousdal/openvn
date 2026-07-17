# Amiga Image Loading and Rendering

M5 PR5.1 adds real image loading paths.

## Generated asset table

The exporter generates:

```text
story/assets.generated.h
story/assets.generated.c
```

Semantic asset IDs are mapped to packaged paths at compile time. The native
runtime does not parse YAML or JSON to resolve assets.

## RTG/DataTypes

The AmigaOS adapter uses:

- `NewDTObject`
- `GID_PICTURE`
- `DrawDTObject`
- `DisposeDTObject`

This allows PNG and other installed picture datatypes on RTG-capable systems.

## Classic ILBM

The portable ILBM loader validates and reads:

- `FORM ILBM`
- `BMHD`
- `CMAP`
- `BODY`

Host tests load real generated ILBM fixtures. On AmigaOS, the loader now
supplies validated ILBM data to the classic rendering path.

Planar BitMap allocation and BODY-to-bitplane conversion follow in M5 PR5.2.

The implementation remains ARexx-first, native, system-friendly and
independent of ACE.
