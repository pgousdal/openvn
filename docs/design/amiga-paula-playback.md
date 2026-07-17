# Amiga four-channel Paula playback

M5 PR6.2 connects the native MOD player to Amiga `audio.device` with one
allocated request pair per Paula channel.

## Playback boundary

The portable `OpenVNPaulaMixer` converts a MOD row note into a voice containing:

- sample start and byte length;
- ProTracker period;
- sample volume, clamped to Paula's 0–64 range;
- optional loop start and loop length;
- a generation counter used to identify retriggers.

This layer has no AmigaOS dependency and is covered by a host-side C test.

## Native scheduling

The Amiga backend allocates channel masks 1, 2, 4 and 8 separately. At tracker
tick zero, every valid row note is mapped to its corresponding Paula channel.
The initial sample is queued once. A valid MOD loop is queued behind it with
`ioa_Cycles = 0`, allowing `audio.device` to repeat the loop continuously.

A new note aborts pending writes for that channel before queueing the new
sample. Stopping music aborts all four channels and the timer request.

## Current limitations

This milestone intentionally implements the basic four-channel path only.
Finetune adjustment, arpeggio, slides, vibrato, volume effects, pattern jumps
and direct custom-chip playback remain future work. Sound effects currently
stop music before using channel zero, avoiding channel arbitration in this
foundation milestone.
