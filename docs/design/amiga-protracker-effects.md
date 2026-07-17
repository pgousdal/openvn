# Amiga ProTracker effect engine

M5 PR6.3 adds persistent per-channel tracker state to the native MOD player.
The player now resolves an effective period and volume on every tracker tick,
separate from the immutable pattern note.

Implemented effects:

- `0xy` arpeggio
- `1xx` period slide up
- `2xx` period slide down
- `Axy` volume slide
- `Bxx` position jump
- `Cxx` set volume
- `Dxx` pattern break
- `Fxx` speed or tempo

Period and volume changes are exposed through `OpenVNMODChannelState` and can
be applied to the Paula mixer without retriggering sample data. More complex
stateful effects, including tone portamento and vibrato, remain later work.
