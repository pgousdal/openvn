# OpenVN Story Format

**Status:** Draft  
**Version:** 0.1.0  
**Schema:** `schemas/openvn-story-0.1.schema.json`

## 1. Purpose

The OpenVN Story Format is the target-independent intermediate representation used between authoring tools and runtime exporters.

The first supported authoring source is Ink. The first supported targets are:

- Ren'Py
- Amiga ARexx

The format MUST remain independent of both targets.

## 2. Design principles

The format:

- represents narrative intent rather than engine code;
- contains no Ren'Py statements;
- contains no ARexx statements;
- uses stable identifiers;
- is deterministic and machine-validatable;
- supports constrained target profiles;
- keeps assets separate from story flow;
- can be generated from Ink and other authoring tools.

## 3. Root object

```json
{
  "format": "openvn-story",
  "version": "0.1",
  "project": {},
  "entry": "intro",
  "characters": {},
  "variables": {},
  "assets": {},
  "scenes": {}
}
```

Required fields:

- `format`
- `version`
- `project`
- `entry`
- `scenes`

## 4. Project metadata

```json
{
  "project": {
    "id": "minimal-demo",
    "title": "Minimal Demo",
    "author": "OpenVN",
    "default_language": "nb"
  }
}
```

`project.id` MUST be a stable machine identifier.

## 5. Characters

```json
{
  "characters": {
    "anna": {
      "name": "Anna",
      "color": "#ffd27f"
    }
  }
}
```

Character IDs are referenced by `say` operations.

## 6. Variables

```json
{
  "variables": {
    "trust_anna": {
      "type": "integer",
      "initial": 0
    },
    "has_key": {
      "type": "boolean",
      "initial": false
    },
    "player_name": {
      "type": "string",
      "initial": "Per"
    }
  }
}
```

Version 0.1 supports:

- `boolean`
- `integer`
- `string`

## 7. Assets

```json
{
  "assets": {
    "backgrounds": {
      "forest": {
        "source": "assets/backgrounds/forest.png"
      }
    },
    "characters": {
      "anna_happy": {
        "source": "assets/characters/anna_happy.png"
      }
    },
    "music": {
      "forest_theme": {
        "source": "assets/music/forest_theme.mod"
      }
    },
    "sfx": {}
  }
}
```

Exporters are responsible for converting assets to target formats where required.

## 8. Scenes

A scene is an ordered list of operations:

```json
{
  "scenes": {
    "intro": {
      "ops": []
    }
  }
}
```

The `entry` field MUST reference an existing scene.

## 9. Operations

### 9.1 Background

```json
{
  "op": "background",
  "asset": "forest"
}
```

### 9.2 Show character

```json
{
  "op": "show",
  "asset": "anna_happy",
  "slot": "center"
}
```

Supported abstract slots in version 0.1:

- `left`
- `center`
- `right`

Targets MAY map slots to profile-specific coordinates.

### 9.3 Hide character

```json
{
  "op": "hide",
  "slot": "center"
}
```

### 9.4 Dialogue

```json
{
  "op": "say",
  "speaker": "anna",
  "text": "Du kom altså."
}
```

Narration omits `speaker`:

```json
{
  "op": "say",
  "text": "Skogen var stille."
}
```

### 9.5 Set variable

```json
{
  "op": "set",
  "variable": "trust_anna",
  "value": 1
}
```

### 9.6 Increment variable

```json
{
  "op": "increment",
  "variable": "trust_anna",
  "amount": 1
}
```

### 9.7 Jump

```json
{
  "op": "jump",
  "target": "forest"
}
```

A jump transfers control to another scene.

### 9.8 Choice

```json
{
  "op": "choice",
  "options": [
    {
      "id": "follow",
      "text": "Bli med Anna",
      "target": "forest"
    },
    {
      "id": "leave",
      "text": "Gå hjem",
      "target": "home"
    }
  ]
}
```

A choice MUST contain at least one option.

Each option MUST contain:

- `id`
- `text`
- `target`

### 9.9 Conditional choice option

```json
{
  "id": "unlock",
  "text": "Lås opp døren",
  "target": "inside",
  "condition": {
    "variable": "has_key",
    "operator": "equals",
    "value": true
  }
}
```

### 9.10 Conditional block

```json
{
  "op": "if",
  "condition": {
    "variable": "trust_anna",
    "operator": "greater_or_equal",
    "value": 2
  },
  "then": [
    {
      "op": "say",
      "speaker": "anna",
      "text": "Jeg stoler på deg."
    }
  ],
  "else": [
    {
      "op": "say",
      "speaker": "anna",
      "text": "Jeg er ikke sikker ennå."
    }
  ]
}
```

Supported operators:

- `equals`
- `not_equals`
- `greater_than`
- `greater_or_equal`
- `less_than`
- `less_or_equal`

### 9.11 Music

```json
{
  "op": "music",
  "asset": "forest_theme",
  "loop": true
}
```

To stop music:

```json
{
  "op": "music_stop"
}
```

### 9.12 Sound effect

```json
{
  "op": "sfx",
  "asset": "door_open"
}
```

### 9.13 End

```json
{
  "op": "end",
  "ending": "followed_anna"
}
```

`ending` is a stable identifier for testing and analytics. Targets do not need to display it.

## 10. Conditions

A condition has this structure:

```json
{
  "variable": "trust_anna",
  "operator": "greater_or_equal",
  "value": 2
}
```

The compared value MUST be compatible with the declared variable type.

## 11. Control-flow rules

- `entry` MUST reference an existing scene.
- Every `jump.target` MUST reference an existing scene.
- Every choice option target MUST reference an existing scene.
- A scene SHOULD terminate with `jump`, `choice`, or `end`.
- Exporters MUST preserve operation order.
- Exporters MUST reject unsupported operations rather than silently omit them.

## 12. Validation levels

Validation SHOULD be performed in three stages:

1. JSON Schema validation
2. semantic validation
3. target-profile validation

Semantic validation includes:

- dangling scene references;
- undeclared characters;
- undeclared variables;
- undeclared assets;
- incompatible variable values;
- duplicate choice IDs within one choice;
- unreachable scenes as warnings.

Target-profile validation includes:

- unsupported image format;
- unsupported screen depth;
- memory budget violations;
- unsupported audio format;
- text length restrictions.

## 13. Target mappings

### 13.1 Ren'Py

Typical mappings:

- scene → Ren'Py label
- say → dialogue statement
- choice → menu
- set/increment → Python store assignment
- jump → Ren'Py jump
- if → Ren'Py conditional
- end → return or final label

### 13.2 Amiga ARexx

Typical mappings:

- scene → ARexx procedure
- say → `OVN_Say`
- choice → `OVN_Choice`
- set/increment → ARexx variables or stems
- jump → scene procedure call or dispatch
- if → ARexx `IF`/`SELECT`
- graphics/audio → OpenVN ARexx SDK calls
- end → `OVN_End`

The OpenVN IR MUST NOT contain target-specific source code.

## 14. Version 0.1 acceptance criteria

A conforming 0.1 implementation can represent and export a story containing:

- project metadata;
- one or more scenes;
- characters;
- declared variables;
- backgrounds;
- character images;
- dialogue;
- one choice;
- conditional behavior;
- music;
- sound effects;
- at least two endings.

The same story file MUST be consumable by both the Ren'Py and Amiga ARexx exporters.
