# OpenVN Compiler Architecture

OpenVN separates narrative authoring, semantic compilation, target export and
runtime execution.

## Pipeline

```text
Ink source
    ↓
Ink parser
    ↓
Semantic story model
    ↓
Validation and diagnostics
    ↓
OpenVN Story Format
    ├── Ren'Py backend
    │     ├── generated script.rpy
    │     ├── generated image/audio declarations
    │     └── copied desktop assets
    │
    └── Amiga backend
          ├── generated story C tables
          ├── generated asset C tables
          ├── profile-specific asset conversion
          ├── ARexx bootstrap
          └── native AmigaOS runtime
```

## Frontend

The frontend reads an OpenVN project and parses its Ink entry file into the
semantic story model.

The parser supports the currently defined OpenVN narrative subset, including:

- text
- choices
- jumps
- endings
- scene changes
- character show/hide operations
- music and sound commands

## Semantic model

The semantic model is independent of any runtime. It represents the story as a
validated graph of typed nodes.

Validation covers:

- duplicate node IDs
- missing entry nodes
- unknown jump and choice targets
- malformed choices
- unreachable nodes
- schema conformance

## OpenVN Story Format

The compiler serializes the validated semantic model to the versioned OpenVN
Story Format.

The JSON document is the portable interchange and debugging representation.
Backends may transform it into a more appropriate runtime representation.

## Ren'Py backend

The Ren'Py backend generates a complete Ren'Py project containing:

- labels and menus
- scene and character commands
- music and sound commands
- image declarations
- audio declarations
- copied assets

## Amiga backend

The Amiga backend is ARexx-first and independent of ACE.

It generates:

- static C story tables
- static C asset tables
- an ARexx bootstrap script
- profile-specific assets
- an Amiga package manifest

The native runtime does not parse story JSON or asset YAML. Immutable story and
asset data are compiled into the player.

The native C layer uses AmigaOS services directly where appropriate, including
Exec, DOS, Intuition, Graphics, Layers, DataTypes and system devices.

## Target profiles

The initial Amiga profiles are:

- `amiga-ocs`
- `amiga-aga`
- `amiga-rtg`

Profiles define display constraints, asset formats and runtime capabilities.

## Design principles

- author once, export to multiple runtimes
- keep narrative semantics backend-independent
- validate before target generation
- generate deterministic output
- respect each target platform
- prefer native AmigaOS integration over generic engine abstraction
