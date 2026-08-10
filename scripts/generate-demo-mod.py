#!/usr/bin/env python3
"""Generate the small, repository-owned canonical ProTracker demo module."""

from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
OUTPUT = ROOT / "examples" / "demo" / "assets" / "music" / "intro.mod"

TITLE = b"OpenVN Demo Melody"
SAMPLE_NAME = b"triangle"
SAMPLE = bytes(
    value & 0xFF
    for value in (
        list(range(-120, 120, 8))
        + list(range(120, -120, -8))
        + [-120, -120, -120, -120]
    )
)
MELODY = (428, 339, 285, 214, 285, 339, 428, 285)


def padded(value: bytes, size: int) -> bytes:
    return value[:size].ljust(size, b"\0")


def note(sample: int, period: int, effect: int = 0, parameter: int = 0) -> bytes:
    return bytes(
        (
            (sample & 0xF0) | ((period >> 8) & 0x0F),
            period & 0xFF,
            ((sample & 0x0F) << 4) | (effect & 0x0F),
            parameter,
        )
    )


def build_module() -> bytes:
    header = bytearray(padded(TITLE, 20))
    header.extend(padded(SAMPLE_NAME, 22))
    header.extend((len(SAMPLE) // 2).to_bytes(2, "big"))
    header.extend((0, 64))  # finetune, volume
    header.extend((0).to_bytes(2, "big"))
    header.extend((len(SAMPLE) // 2).to_bytes(2, "big"))
    header.extend(bytes(30 * 30))
    header.extend((1, 0))  # song length, restart position
    header.extend(bytes(128))
    header.extend(b"M.K.")

    pattern = bytearray(64 * 4 * 4)
    for index, period in enumerate(MELODY):
        offset = index * 8 * 4 * 4
        pattern[offset : offset + 4] = note(1, period)

    return bytes(header + pattern + SAMPLE)


def main() -> None:
    OUTPUT.write_bytes(build_module())
    print(OUTPUT)


if __name__ == "__main__":
    main()
