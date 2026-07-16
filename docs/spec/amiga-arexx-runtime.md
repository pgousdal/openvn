# OpenVN Amiga ARexx Runtime Specification

**Status:** Draft  
**Version:** 0.1.0  
**Target:** Classic Amiga systems with ARexx  
**Runtime model:** ARexx-first, minimal native player  
**Normative language:** The terms **MUST**, **MUST NOT**, **SHOULD**, **SHOULD NOT**, and **MAY** are to be interpreted as requirements levels.

---

## 1. Purpose

This document specifies the OpenVN runtime architecture for Amiga systems.

The runtime is intentionally divided into two layers:

1. an ARexx runtime and standard library containing all practical visual-novel logic; and
2. a minimal native OpenVN Player providing only services that require direct AmigaOS, hardware, graphics, audio, input, timing, or memory access.

The central design rule is:

> Everything that can reasonably be implemented in ARexx MUST be implemented in ARexx. Native C code MUST be limited to functionality that cannot be implemented correctly, efficiently, or portably in ARexx.

The OpenVN Player is not a visual-novel engine. It is a small multimedia and operating-system service host controlled through an ARexx port.

---

## 2. Goals

The Amiga ARexx runtime SHALL:

- make ARexx the primary runtime language;
- keep the native player small and auditable;
- preserve readable and modifiable generated game scripts;
- support OpenVN stories without embedding story logic in C;
- provide a stable command ABI between ARexx and the player;
- allow the ARexx SDK to evolve independently of the native player;
- support development on emulators and real Amiga hardware;
- permit future native acceleration without changing game-facing APIs;
- support both ECS/OCS-oriented and more capable Amiga profiles;
- make debugging possible without recompiling the player.

---

## 3. Non-goals

Version 0.1 does not attempt to:

- implement a full Ren'Py-compatible runtime;
- execute arbitrary Ink directly on the Amiga;
- place scene graphs, dialogue trees, inventory systems, or save-game policy in C;
- provide a general-purpose GUI toolkit;
- require Workbench integration for gameplay;
- provide video playback;
- support arbitrary modern image or audio formats;
- hide or obfuscate generated ARexx source;
- optimize every effect before measuring it on target hardware.

---

## 4. Runtime architecture

```text
Ink source
    |
    v
OpenVN compiler
    |
    v
OpenVN intermediate representation
    |
    v
Amiga ARexx exporter
    |
    +-- game ARexx scripts
    +-- OpenVN ARexx SDK
    +-- converted assets
    +-- game manifest
    |
    v
OpenVN Player
    |
    +-- ARexx message port
    +-- display and bitmap services
    +-- text rendering
    +-- audio playback
    +-- input services
    +-- timing services
    +-- restricted file services
```

The exporter generates ARexx code that calls the OpenVN ARexx SDK.

The SDK implements visual-novel behavior by sending primitive commands to the native player.

The player executes commands and returns results through the standard ARexx host mechanism.

---

## 5. Responsibility boundary

### 5.1 ARexx responsibilities

The ARexx layer MUST own all practical game and visual-novel logic, including:

- story flow;
- scene procedures;
- labels and jumps;
- dialogue sequencing;
- speaker state;
- choices;
- conditional choices;
- variables and flags;
- arithmetic and comparisons;
- inventory;
- relationship values;
- achievements;
- menus;
- backlog logic;
- save-game serialization;
- load-game restoration;
- localization selection;
- typewriter behavior;
- transitions composed from primitive drawing operations;
- input interpretation;
- game configuration;
- debugging and trace logging;
- game-specific logic;
- fallback behavior;
- high-level resource naming;
- validation of game-level parameters.

### 5.2 Native player responsibilities

The native player MAY own only services requiring native implementation, including:

- opening and closing screens or windows;
- allocating and releasing bitmaps;
- loading supported image data;
- palette operations;
- blitting and compositing;
- text rasterization;
- font loading where supported;
- audio and module playback;
- keyboard, mouse, and joystick access;
- timing and tick counters;
- controlled file access;
- memory accounting;
- decompression;
- ARexx host-port management;
- conversion of command results to ARexx return values;
- reporting native errors.

### 5.3 Prohibited native responsibilities

The player MUST NOT implement game-level concepts such as:

- scenes;
- dialogue trees;
- characters as narrative entities;
- inventories;
- relationship systems;
- achievements;
- story variables;
- branching story logic;
- save-slot policy;
- localization policy;
- visual-novel menus;
- OpenVN story interpretation.

A native command such as `CHOICE`, `SAY`, `START_SCENE`, `SET_FLAG`, or `SAVE_GAME` is therefore outside the 0.1 ABI.

---

## 6. ARexx host port

### 6.1 Port name

The player MUST publish an ARexx port named:

```text
OPENVN
```

If this name is unavailable, the player MAY publish a unique suffixed port such as:

```text
OPENVN.1
OPENVN.2
```

The effective port name MUST be made available to the launcher script.

### 6.2 Addressing

ARexx scripts communicate with the player using:

```rexx
ADDRESS OPENVN
```

or an equivalent dynamically selected port name.

### 6.3 Command model

Commands are textual and synchronous in version 0.1.

A command consists of:

```text
NAMESPACE.ACTION key=value key=value ...
```

Example:

```text
SCREEN.OPEN width=320 height=256 depth=5 mode=PAL
```

The command name is case-insensitive.

Argument keys are case-insensitive.

String values requiring spaces or special characters MUST be quoted.

### 6.4 Result model

Each command returns:

- an ARexx return code;
- an optional textual result;
- an optional secondary error code exposed through the normal ARexx host mechanism where available.

Successful commands SHOULD return code `0`.

Commands returning data place the result in `RESULT`.

Example:

```rexx
ADDRESS OPENVN
'SYSTEM.VERSION'
version = RESULT
```

---

## 7. Common data rules

### 7.1 Integers

Integers are base-10 signed values unless a command explicitly states otherwise.

### 7.2 Booleans

Boolean arguments use:

```text
0
1
```

The values `false` and `true` MAY be accepted as aliases.

### 7.3 Strings

Strings containing whitespace, quotation marks, semicolons, or equals signs MUST be quoted.

The SDK MUST provide a safe quoting helper.

### 7.4 Identifiers

Resource identifiers:

- MUST begin with a letter or underscore;
- MAY contain letters, digits, underscores, hyphens, and periods;
- MUST be unique within their resource type;
- SHOULD be generated deterministically by the exporter.

### 7.5 Paths

Paths are relative to the game root unless explicitly documented.

Paths MUST NOT escape the game root through `..` traversal.

The player MUST reject invalid traversal attempts.

### 7.6 Coordinates

Unless otherwise specified:

- origin is the upper-left corner;
- x increases to the right;
- y increases downward;
- units are physical pixels.

### 7.7 Colors

Version 0.1 uses palette indices for indexed display modes.

True-color values are outside the required MVP.

---

## 8. Required command namespaces

The version 0.1 player ABI contains these namespaces:

- `SYSTEM`
- `SCREEN`
- `PALETTE`
- `IMAGE`
- `TEXT`
- `AUDIO`
- `INPUT`
- `TIME`
- `FILE`

Implementations MAY add experimental namespaces, but generated games MUST NOT depend on them unless declared by the selected target profile.

---

## 9. SYSTEM commands

### 9.1 `SYSTEM.VERSION`

Returns the player ABI version.

```text
SYSTEM.VERSION
```

Example result:

```text
OPENVN-PLAYER/0.1
```

### 9.2 `SYSTEM.CAPABILITIES`

Returns a space-separated list of supported capability identifiers.

```text
SYSTEM.CAPABILITIES
```

Example:

```text
screen.indexed image.ilbm text.bitmap audio.mod input.keyboard file.sandbox
```

### 9.3 `SYSTEM.LAST_ERROR`

Returns the most recent human-readable native error message for the calling context.

```text
SYSTEM.LAST_ERROR
```

### 9.4 `SYSTEM.MEMORY`

Returns available memory information where supported.

```text
SYSTEM.MEMORY
```

Suggested result:

```text
chip_free=3145728 fast_free=8388608
```

This command is advisory and MUST NOT be required for ordinary game flow.

### 9.5 `SYSTEM.QUIT`

Requests a clean player shutdown.

```text
SYSTEM.QUIT
```

The player MUST release resources and close its ARexx port.

---

## 10. SCREEN commands

### 10.1 `SCREEN.OPEN`

Opens the gameplay display.

```text
SCREEN.OPEN width=<int> height=<int> depth=<int> mode=<PAL|NTSC|AUTO>
```

Required arguments:

- `width`
- `height`
- `depth`

Optional arguments:

- `mode`
- `title`
- `windowed`
- `doublebuffer`

Version 0.1 implementations MUST support at least one target-profile-defined screen configuration.

### 10.2 `SCREEN.CLOSE`

Closes the current display.

```text
SCREEN.CLOSE
```

### 10.3 `SCREEN.CLEAR`

Clears the display or specified layer.

```text
SCREEN.CLEAR [color=<palette-index>] [layer=<id>]
```

### 10.4 `SCREEN.PRESENT`

Makes pending drawing operations visible.

```text
SCREEN.PRESENT
```

On single-buffered implementations this MAY be a no-op.

### 10.5 `SCREEN.INFO`

Returns the effective display configuration.

```text
SCREEN.INFO
```

Suggested result:

```text
width=320 height=256 depth=5 mode=PAL doublebuffer=1
```

---

## 11. PALETTE commands

### 11.1 `PALETTE.LOAD`

Loads a palette from a supported file or image resource.

```text
PALETTE.LOAD file="<path>"
```

or:

```text
PALETTE.LOAD image=<image-id>
```

### 11.2 `PALETTE.SET`

Sets a palette entry.

```text
PALETTE.SET index=<int> r=<int> g=<int> b=<int>
```

Component ranges are implementation-defined by the profile but SHOULD accept 0–255 and convert internally.

### 11.3 `PALETTE.FADE`

Optional accelerated primitive:

```text
PALETTE.FADE target=<black|current|file> steps=<int> ticks=<int>
```

This command is OPTIONAL in 0.1.

Fade policy remains in ARexx. The native command, when available, only accelerates palette stepping.

---

## 12. IMAGE commands

### 12.1 `IMAGE.LOAD`

Loads an image into a named native resource.

```text
IMAGE.LOAD id=<image-id> file="<path>" [format=<ILBM|AUTO>]
```

The MVP MUST support target-profile-approved ILBM assets.

### 12.2 `IMAGE.FREE`

Releases an image resource.

```text
IMAGE.FREE id=<image-id>
```

### 12.3 `IMAGE.DRAW`

Draws an image.

```text
IMAGE.DRAW id=<image-id> x=<int> y=<int> [layer=<id>] [mask=<0|1>]
```

### 12.4 `IMAGE.DRAW_REGION`

Draws part of an image.

```text
IMAGE.DRAW_REGION id=<image-id> sx=<int> sy=<int> width=<int> height=<int> x=<int> y=<int> [layer=<id>]
```

This command is RECOMMENDED but not required for the smallest MVP.

### 12.5 `IMAGE.CLEAR`

Clears an image layer or a rectangular region.

```text
IMAGE.CLEAR [layer=<id>] [x=<int> y=<int> width=<int> height=<int>] [color=<palette-index>]
```

### 12.6 `IMAGE.INFO`

Returns image dimensions and format.

```text
IMAGE.INFO id=<image-id>
```

Suggested result:

```text
width=320 height=128 depth=5 format=ILBM
```

### 12.7 Narrative neutrality

The player MUST NOT expose narrative commands such as:

```text
SHOW_BACKGROUND
SHOW_CHARACTER
HIDE_CHARACTER
```

The ARexx SDK MAY expose these as convenience functions implemented using `IMAGE.*`.

---

## 13. TEXT commands

### 13.1 `TEXT.FONT_LOAD`

Loads a font resource.

```text
TEXT.FONT_LOAD id=<font-id> file="<path>" [size=<int>]
```

A built-in player font MAY be used where external font loading is unavailable.

### 13.2 `TEXT.FONT_FREE`

Releases a font resource.

```text
TEXT.FONT_FREE id=<font-id>
```

### 13.3 `TEXT.MEASURE`

Measures text without drawing it.

```text
TEXT.MEASURE font=<font-id> text="<string>" [maxwidth=<int>]
```

Suggested result:

```text
width=144 height=16 lines=1
```

### 13.4 `TEXT.DRAW`

Draws text.

```text
TEXT.DRAW font=<font-id> x=<int> y=<int> text="<string>" [color=<palette-index>] [maxwidth=<int>] [layer=<id>]
```

### 13.5 `TEXT.CLEAR`

Clears text from a layer or region.

```text
TEXT.CLEAR [layer=<id>] [x=<int> y=<int> width=<int> height=<int>] [color=<palette-index>]
```

### 13.6 `TEXT.GLYPH`

Optional low-level glyph drawing command.

```text
TEXT.GLYPH font=<font-id> x=<int> y=<int> code=<int> [color=<palette-index>] [layer=<id>]
```

This command MAY be added for efficient typewriter effects but is not required in 0.1.

### 13.7 Text wrapping

High-level wrapping policy SHOULD be implemented in ARexx.

The player MAY support `maxwidth` as a native acceleration primitive.

---

## 14. AUDIO commands

### 14.1 `AUDIO.MOD_LOAD`

Loads a module.

```text
AUDIO.MOD_LOAD id=<audio-id> file="<path>"
```

### 14.2 `AUDIO.MOD_PLAY`

Starts module playback.

```text
AUDIO.MOD_PLAY id=<audio-id> [loop=<0|1>] [volume=<int>]
```

### 14.3 `AUDIO.MOD_STOP`

Stops module playback.

```text
AUDIO.MOD_STOP [fade_ticks=<int>]
```

Fade policy remains controlled by ARexx.

### 14.4 `AUDIO.SFX_LOAD`

Loads a sound effect.

```text
AUDIO.SFX_LOAD id=<audio-id> file="<path>"
```

### 14.5 `AUDIO.SFX_PLAY`

Plays a sound effect.

```text
AUDIO.SFX_PLAY id=<audio-id> [channel=<int>] [volume=<int>] [priority=<int>]
```

### 14.6 `AUDIO.FREE`

Releases a loaded audio resource.

```text
AUDIO.FREE id=<audio-id>
```

### 14.7 Audio policy

Music selection, cue sequencing, fades, and game-state relationships MUST remain in ARexx.

---

## 15. INPUT commands

### 15.1 `INPUT.POLL`

Returns the current input state without blocking.

```text
INPUT.POLL
```

Suggested result:

```text
type=key code=32 ascii=32 qualifier=0
```

If no event is pending:

```text
type=none
```

### 15.2 `INPUT.WAIT`

Waits for an input event.

```text
INPUT.WAIT [types=<key,mouse,joystick>] [timeout_ticks=<int>]
```

Suggested keyboard result:

```text
type=key code=32 ascii=32 qualifier=0
```

Suggested mouse result:

```text
type=mouse button=1 x=132 y=204
```

Suggested timeout result:

```text
type=timeout
```

### 15.3 `INPUT.FLUSH`

Clears pending input events.

```text
INPUT.FLUSH
```

### 15.4 Interpretation

The player reports primitive events only.

Concepts such as “advance dialogue,” “choose option,” “open menu,” and “skip” MUST be interpreted by the ARexx SDK.

---

## 16. TIME commands

### 16.1 `TIME.TICKS`

Returns the current monotonic tick count.

```text
TIME.TICKS
```

### 16.2 `TIME.WAIT`

Blocks for a number of ticks.

```text
TIME.WAIT ticks=<int>
```

### 16.3 `TIME.FREQUENCY`

Returns ticks per second.

```text
TIME.FREQUENCY
```

The SDK MUST avoid assuming a fixed PAL or NTSC frequency.

---

## 17. FILE commands

### 17.1 General restrictions

Player file access MUST be sandboxed to approved game directories.

The player MUST reject:

- path traversal outside approved roots;
- writes to asset directories unless explicitly permitted;
- absolute paths not enabled by the target profile.

### 17.2 `FILE.EXISTS`

```text
FILE.EXISTS path="<path>"
```

Returns `1` or `0`.

### 17.3 `FILE.READ`

```text
FILE.READ path="<path>"
```

Returns text content within implementation-defined size limits.

The SDK SHOULD use this command only for small configuration and save files.

### 17.4 `FILE.WRITE`

```text
FILE.WRITE path="<path>" data="<string>" [append=<0|1>]
```

Writes text data to an allowed writable location.

### 17.5 `FILE.DELETE`

```text
FILE.DELETE path="<path>"
```

This command MAY be disabled by profiles.

### 17.6 `FILE.LIST`

```text
FILE.LIST path="<directory>" [pattern="<pattern>"]
```

This command is OPTIONAL in 0.1.

### 17.7 Save-game policy

The player provides only restricted file primitives.

Save slots, serialization format, checksums, migration, and restoration logic MUST be implemented by the ARexx SDK.

---

## 18. Return codes

The following return-code model is recommended:

| Code | Meaning |
|---:|---|
| 0 | Success |
| 5 | Warning or partial result |
| 10 | Invalid command or argument |
| 15 | Resource or state error |
| 20 | Fatal player error |

Suggested native error identifiers include:

```text
OVN_OK
OVN_WARN_FALLBACK
OVN_ERR_UNKNOWN_COMMAND
OVN_ERR_INVALID_ARGUMENT
OVN_ERR_MISSING_ARGUMENT
OVN_ERR_UNSUPPORTED
OVN_ERR_NOT_FOUND
OVN_ERR_ALREADY_EXISTS
OVN_ERR_INVALID_STATE
OVN_ERR_OUT_OF_MEMORY
OVN_ERR_IO
OVN_ERR_FORMAT
OVN_ERR_PATH_DENIED
OVN_ERR_DISPLAY
OVN_ERR_AUDIO
OVN_ERR_INTERNAL
```

Human-readable details SHOULD be available through `SYSTEM.LAST_ERROR`.

---

## 19. Capability discovery

The ARexx SDK MUST query `SYSTEM.VERSION` and SHOULD query `SYSTEM.CAPABILITIES` at startup.

The SDK MUST fail clearly when a required profile capability is unavailable.

Example required capabilities for the first profile:

```text
screen.indexed
image.ilbm
text.bitmap
audio.mod
input.keyboard
time.ticks
file.sandbox
```

Optional features MUST have ARexx fallbacks where practical.

---

## 20. OpenVN ARexx SDK

The SDK is the actual Amiga visual-novel runtime.

Recommended modules:

```text
runtime/
├── openvn.rexx
├── system.rexx
├── resources.rexx
├── graphics.rexx
├── text.rexx
├── dialogue.rexx
├── choices.rexx
├── scenes.rexx
├── variables.rexx
├── savegame.rexx
├── localisation.rexx
├── menus.rexx
├── backlog.rexx
├── transitions.rexx
├── input.rexx
└── debug.rexx
```

### 20.1 SDK responsibilities

The SDK MUST provide high-level functions for:

- runtime initialization and shutdown;
- resource lookup;
- background display;
- character sprite display;
- dialogue boxes;
- speaker labels;
- wrapped text;
- typewriter text;
- advance input;
- choices;
- scene calls and jumps;
- variables and flags;
- save and load;
- localization;
- menus;
- error reporting.

### 20.2 Example SDK API

```rexx
CALL OVN_Init
CALL OVN_Background "forest"
CALL OVN_ShowCharacter "anna", "happy", 160, 32
CALL OVN_Say "Anna", "Du kom altså."
choice = OVN_Choice(choice.)
CALL OVN_PlayMusic "forest_theme", 1
CALL OVN_Save 1
CALL OVN_Shutdown
```

These functions are not native player commands.

They are ARexx procedures built from the primitive ABI.

---

## 21. Generated game structure

The exporter SHOULD generate:

```text
build/amiga-arexx/
├── start.rexx
├── game.rexx
├── config.rexx
├── manifest.rexx
├── characters.rexx
├── variables.rexx
├── scenes/
│   ├── intro.rexx
│   ├── forest.rexx
│   └── ending.rexx
├── localisation/
│   ├── en.rexx
│   └── nb.rexx
├── runtime/
│   └── *.rexx
├── assets/
│   ├── backgrounds/
│   ├── characters/
│   ├── fonts/
│   ├── music/
│   └── sfx/
├── saves/
└── OpenVNPlayer
```

The exact executable name MAY vary by platform packaging conventions.

---

## 22. Launcher behavior

`start.rexx` SHOULD:

1. determine the game root;
2. start the OpenVN Player if it is not already running;
3. determine the effective ARexx port name;
4. set `ADDRESS` to that port;
5. verify ABI compatibility;
6. load runtime modules;
7. load game configuration;
8. initialize the screen and resources;
9. call the initial scene;
10. perform orderly shutdown.

Illustrative skeleton:

```rexx
/* start.rexx */

SIGNAL ON ERROR
SIGNAL ON FAILURE
SIGNAL ON HALT

gameRoot = GET_GAME_ROOT()

CALL StartPlayer gameRoot
portName = FindOpenVNPort()

ADDRESS VALUE portName

'SYSTEM.VERSION'
playerVersion = RESULT

CALL LoadRuntime gameRoot
CALL OVN_Init gameRoot, playerVersion
CALL Scene_intro
CALL OVN_Shutdown

EXIT 0

ERROR:
    CALL OVN_ReportError "ARexx error", RC
    EXIT 10

FAILURE:
    CALL OVN_ReportError "Player command failed", RC
    EXIT 15

HALT:
    CALL OVN_Shutdown
    EXIT 20
```

---

## 23. Scene representation

Each scene SHOULD compile to an ARexx procedure.

Example:

```rexx
Scene_intro: PROCEDURE EXPOSE OVN.
    CALL OVN_Background "forest"
    CALL OVN_PlayMusic "forest_theme", 1
    CALL OVN_ShowCharacter "anna", "happy", 156, 28

    CALL OVN_Say "Anna", "Du kom altså."
    CALL OVN_Say OVN.playerName, "Jeg lovet at jeg skulle komme."

    choices.0 = 2
    choices.1.id = "follow"
    choices.1.text = "Bli med Anna"
    choices.1.enabled = 1

    choices.2.id = "leave"
    choices.2.text = "Gå tilbake"
    choices.2.enabled = 1

    selected = OVN_Choice(choices.)

    SELECT
        WHEN selected = "follow" THEN DO
            OVN.trust_anna = OVN.trust_anna + 1
            CALL Scene_forest
        END

        WHEN selected = "leave" THEN
            CALL Scene_home

        OTHERWISE
            CALL OVN_Fatal "Invalid choice result: " || selected
    END

RETURN
```

---

## 24. Typewriter behavior

Typewriter display MUST initially be implemented in ARexx.

A simple implementation may repeatedly call `TEXT.DRAW` or `TEXT.GLYPH`.

The SDK SHOULD support:

- configurable character delay;
- immediate completion on advance input;
- disabling the effect;
- punctuation pauses;
- text sound hooks;
- profile-specific acceleration.

A native accelerated primitive MAY be introduced later only after profiling.

The game-facing SDK API MUST remain stable if acceleration is added.

---

## 25. Choice behavior

Choices MUST be implemented in ARexx.

The SDK is responsible for:

- filtering disabled choices;
- laying out option text;
- cursor movement;
- keyboard and mouse interpretation;
- scrolling;
- selection confirmation;
- returning the selected choice identifier;
- cancellation policy;
- debug display.

The player only draws text and reports primitive input events.

---

## 26. Save-game model

### 26.1 Ownership

ARexx owns save-game state and serialization.

### 26.2 Minimum saved state

A save SHOULD include:

- save-format version;
- game identifier;
- game version;
- current scene;
- resume label or checkpoint;
- global variables;
- flags;
- inventory;
- relationship values;
- selected language;
- optional playtime;
- optional timestamp.

### 26.3 Suggested text format

```text
OPENVN_SAVE=1
game_id=openvn_demo
game_version=0.1.0
scene=forest
checkpoint=after_first_choice
language=nb
player_name=Per
trust_anna=3
has_key=1
```

### 26.4 Restrictions

Arbitrary ARexx source MUST NOT be loaded from save files.

Save values MUST be parsed as data.

---

## 27. Localization

Localization policy belongs to the ARexx SDK.

Generated localization modules MAY use stem variables:

```rexx
L.dialogue_intro_001 = "Du kom altså."
L.choice_follow = "Bli med Anna"
L.choice_leave = "Gå tilbake"
```

The exporter SHOULD produce stable localization keys.

The player receives only resolved strings.

---

## 28. Debugging

The SDK SHOULD support debug modes for:

- command tracing;
- scene entry and exit;
- variable changes;
- choice results;
- resource loading;
- save/load operations;
- player errors;
- timing measurements.

Example:

```rexx
CALL OVN_Debug "Entering Scene_intro"
```

The player MAY expose low-level diagnostics, but game-level trace formatting belongs in ARexx.

---

## 29. Performance policy

All new behavior follows this sequence:

1. implement in ARexx;
2. test correctness;
3. profile on representative target hardware;
4. identify the actual bottleneck;
5. add the smallest possible native primitive;
6. retain the existing SDK interface;
7. retain an ARexx fallback when practical.

Convenience alone is not sufficient reason to add a native command.

A native implementation MUST have a documented justification.

---

## 30. Target profiles

The runtime ABI is shared, while target profiles define limits.

Example profile fields:

```yaml
id: amiga-ocs
screen:
  width: 320
  height: 256
  depth: 5
  mode: PAL
images:
  formats:
    - ilbm
audio:
  music:
    - mod
  sfx:
    - 8svx
memory:
  chip_min_kib: 1024
  fast_recommended_kib: 2048
runtime:
  arexx_required: true
```

Profiles MAY define:

- screen modes;
- color limits;
- asset formats;
- maximum resource dimensions;
- memory budgets;
- audio channel policies;
- supported optional commands;
- fallback expectations.

---

## 31. MVP command set

The smallest useful player MUST implement:

```text
SYSTEM.VERSION
SYSTEM.CAPABILITIES
SYSTEM.LAST_ERROR
SYSTEM.QUIT

SCREEN.OPEN
SCREEN.CLOSE
SCREEN.CLEAR
SCREEN.PRESENT

PALETTE.LOAD

IMAGE.LOAD
IMAGE.FREE
IMAGE.DRAW
IMAGE.CLEAR

TEXT.FONT_LOAD
TEXT.MEASURE
TEXT.DRAW
TEXT.CLEAR

AUDIO.MOD_LOAD
AUDIO.MOD_PLAY
AUDIO.MOD_STOP
AUDIO.SFX_LOAD
AUDIO.SFX_PLAY
AUDIO.FREE

INPUT.WAIT
INPUT.POLL
INPUT.FLUSH

TIME.TICKS
TIME.WAIT
TIME.FREQUENCY

FILE.EXISTS
FILE.READ
FILE.WRITE
```

The first implementation MAY temporarily omit sound effects if module playback and the rest of the vertical slice are complete.

---

## 32. First vertical slice

The first end-to-end demonstrator MUST perform:

1. launch the player;
2. connect through the `OPENVN` ARexx port;
3. verify `SYSTEM.VERSION`;
4. open a 320×256 indexed screen;
5. load and display one ILBM background;
6. draw a dialogue box using image or fill primitives;
7. draw a speaker name;
8. draw wrapped dialogue text;
9. wait for keyboard or mouse advance;
10. display two choices;
11. accept a selection;
12. show a second line based on that selection;
13. play and stop one MOD;
14. write a small save file;
15. read the save file back;
16. close the display;
17. shut down cleanly.

No story-state logic for this slice may be implemented in C.

---

## 33. Acceptance criteria for version 0.1

The runtime specification is considered implemented when:

- the player publishes a working ARexx port;
- the required MVP commands are documented and implemented;
- an ARexx SDK wraps the primitive command ABI;
- one generated demonstration story runs without native story logic;
- dialogue and choices are implemented in ARexx;
- variables and branching are implemented in ARexx;
- save and load are implemented in ARexx using restricted file primitives;
- the same player binary can run at least two different generated stories;
- player commands return consistent error codes;
- invalid file traversal is rejected;
- the repository contains tests for the command parser;
- the repository contains an emulator or host-side test strategy;
- all native extensions beyond the MVP include a written performance justification.

---

## 34. Testing strategy

### 34.1 Host-side tests

Where practical, host-side tests SHOULD cover:

- command tokenization;
- quoted arguments;
- argument validation;
- missing arguments;
- unknown commands;
- return-code mapping;
- path sandboxing;
- resource identifier validation;
- capability output;
- error-message storage.

### 34.2 ARexx SDK tests

SDK tests SHOULD cover:

- quote escaping;
- dialogue composition;
- line wrapping;
- typewriter cancellation;
- choice filtering;
- choice navigation;
- save serialization;
- save parsing;
- localization lookup;
- scene dispatch;
- error propagation.

### 34.3 Integration tests

Integration tests SHOULD run under an Amiga emulator and verify the first vertical slice.

Golden screenshots MAY be used for stable rendering cases.

### 34.4 Real-hardware tests

At least one representative low-end profile SHOULD be tested on real hardware or cycle-accurate emulation before declaring performance-sensitive features complete.

---

## 35. Security and robustness

The player MUST:

- validate every native command argument;
- reject paths outside configured roots;
- avoid fixed-size unchecked string copies;
- handle missing assets without corrupting runtime state;
- release resources on shutdown;
- report out-of-memory failures;
- avoid executing file contents as commands;
- avoid treating save files as ARexx source;
- tolerate malformed commands without crashing;
- leave the display in a recoverable state after fatal errors where possible.

The SDK SHOULD present a clear fatal-error screen when ordinary rendering remains available.

---

## 36. Versioning

The player ABI uses semantic-style versions:

```text
OPENVN-PLAYER/MAJOR.MINOR
```

Rules:

- a major version change may break commands;
- a minor version change may add backward-compatible commands or arguments;
- generated games MUST declare the minimum required ABI;
- the SDK MUST verify compatibility before gameplay starts.

The ARexx SDK has a separate version:

```text
OPENVN-AREXX-SDK/MAJOR.MINOR.PATCH
```

---

## 37. Future extensions

Possible future native primitives include:

- accelerated masked blits;
- sprite-channel support;
- native palette fades;
- image decompression;
- text glyph caching;
- region-based redraw;
- asynchronous audio status;
- asynchronous input event queues;
- packed resource archives;
- CD audio;
- RTG display support;
- AHI support.

These are not approval to move game logic into C.

Each extension must remain a primitive service.

---

## 38. Design decision summary

The Amiga runtime is defined by these decisions:

1. ARexx is the runtime language.
2. The native player is a service host, not a story engine.
3. Game logic remains readable and modifiable.
4. High-level APIs belong to the ARexx SDK.
5. Native commands remain primitive and narrative-neutral.
6. Save, choices, menus, variables, and scene flow remain in ARexx.
7. Native acceleration is added only after profiling.
8. Game-facing SDK APIs remain stable when acceleration is introduced.
9. The player and SDK are versioned separately.
10. The same minimal player must run multiple OpenVN games.

---

## Appendix A: Minimal ARexx example

```rexx
/* demo.rexx */

SIGNAL ON ERROR
SIGNAL ON FAILURE

ADDRESS OPENVN

'SYSTEM.VERSION'
SAY "Player:" RESULT

'SCREEN.OPEN width=320 height=256 depth=5 mode=PAL'
'PALETTE.LOAD file="assets/backgrounds/forest.iff"'

'IMAGE.LOAD id=forest file="assets/backgrounds/forest.iff"'
'IMAGE.DRAW id=forest x=0 y=0 layer=background'

'TEXT.FONT_LOAD id=main file="assets/fonts/topaz.font" size=8'
'TEXT.DRAW font=main x=16 y=190 color=1 text="Anna"'
'TEXT.DRAW font=main x=16 y=206 color=1 maxwidth=288 text="Du kom altså."'

'SCREEN.PRESENT'
'INPUT.WAIT types=key,mouse'
event = RESULT

'FILE.WRITE path="saves/demo.sav" data="scene=intro"'
'FILE.READ path="saves/demo.sav"'
SAY "Save:" RESULT

'SYSTEM.QUIT'
EXIT 0

ERROR:
    SAY "ARexx error:" RC
    EXIT 10

FAILURE:
    'SYSTEM.LAST_ERROR'
    SAY "Player error:" RESULT
    EXIT 15
```

---

## Appendix B: Example high-level SDK function

```rexx
OVN_Say: PROCEDURE EXPOSE OVN.
    PARSE ARG speaker, text

    CALL OVN_DrawDialogueFrame

    ADDRESS VALUE OVN.port
    'TEXT.CLEAR layer=dialogue'
    'TEXT.DRAW font=' || OVN.font || ,
        'x=' || OVN.speakerX || ,
        'y=' || OVN.speakerY || ,
        'color=' || OVN.speakerColor || ,
        'text=' || OVN_Quote(speaker)

    CALL OVN_Typewriter text
    CALL OVN_WaitAdvance

RETURN
```

---

## Appendix C: Native-command review checklist

Before adding a C command, answer:

1. Can this be implemented correctly in ARexx?
2. Is the ARexx implementation measurably too slow?
3. On which target hardware was it measured?
4. What exact primitive is the bottleneck?
5. Can a smaller native primitive solve it?
6. Can the existing SDK API remain unchanged?
7. Is an ARexx fallback possible?
8. Is the command narrative-neutral?
9. Is the command covered by parser and integration tests?
10. Is the justification documented?

If these questions do not support a native implementation, the feature remains in ARexx.
