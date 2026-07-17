# Native ProTracker MOD Replayer Foundation

M5 PR6.1 adds a native MOD parser and deterministic playback state machine.

## Parser

The runtime supports four-channel ProTracker modules with signatures:

- `M.K.`
- `M!K!`
- `4CHN`
- `FLT4`

It parses:

- song title
- 31 sample headers
- order table
- pattern notes
- sample data

## Playback state

The portable player tracks:

- order
- row
- tick
- speed
- BPM
- looping state

Initial effect support includes `Fxx` speed and tempo changes.

## AmigaOS scheduling

The native adapter uses `timer.device` to schedule tracker ticks. It exposes
the row/channel boundary required for Paula playback.

M5 PR6.2 will add separate audio requests for all four Paula channels and
actual simultaneous sample triggering, volume and loop updates.

## Host verification

Host tests parse a real minimal MOD fixture, verify its first note and
advance the state machine through one full tracker row.

The implementation remains ARexx-first, native, system-friendly and
independent of ACE.
