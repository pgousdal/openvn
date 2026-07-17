# Complete ProTracker effect playback

M5 PR6.4 extends the portable MOD player with persistent per-channel state for tone portamento, vibrato, tremolo, sample offsets, combined effects, and the most important `Exx` timing commands.

Supported commands are `0xy`, `1xx`, `2xx`, `3xx`, `4xy`, `5xy`, `6xy`, `7xy`, `9xx`, `Axy`, `Bxx`, `Cxx`, `Dxx`, and `Fxx`. Extended commands include fine period slides, pattern loop, retrigger, fine volume slides, note cut, note delay, and pattern delay.

The player exposes effective period, volume, trigger state, and sample offset through `OpenVNMODChannelState`. The Paula backend can therefore update active hardware channels without restarting samples unless `triggered` is set.
