from __future__ import annotations

import struct
import wave
from pathlib import Path

from PIL import Image

from ...errors import OpenVNError
from .profiles import AmigaProfile


def convert_png_to_ilbm(
    source: str | Path,
    destination: str | Path,
    *,
    colors: int,
) -> Path:
    src = Path(source)
    dst = Path(destination)
    dst.parent.mkdir(parents=True, exist_ok=True)

    image = Image.open(src).convert("P", palette=Image.Palette.ADAPTIVE, colors=colors)
    width, height = image.size
    palette = image.getpalette()[: colors * 3]
    palette += [0] * (colors * 3 - len(palette))
    pixels = image.tobytes()

    bmhd = struct.pack(
        ">HHhhBBBBHBBhh",
        width,
        height,
        0,
        0,
        (colors - 1).bit_length(),
        0,
        0,
        0,
        0,
        10,
        11,
        width,
        height,
    )
    cmap = bytes(palette)
    body = pixels

    chunks = []
    for chunk_id, payload in ((b"BMHD", bmhd), (b"CMAP", cmap), (b"BODY", body)):
        padded = payload + (b"\0" if len(payload) % 2 else b"")
        chunks.append(chunk_id + struct.pack(">I", len(payload)) + padded)

    form_payload = b"ILBM" + b"".join(chunks)
    dst.write_bytes(b"FORM" + struct.pack(">I", len(form_payload)) + form_payload)
    return dst


def convert_wav_to_8svx(source: str | Path, destination: str | Path) -> Path:
    src = Path(source)
    dst = Path(destination)
    dst.parent.mkdir(parents=True, exist_ok=True)

    with wave.open(str(src), "rb") as wav:
        channels = wav.getnchannels()
        sample_width = wav.getsampwidth()
        frame_rate = wav.getframerate()
        frames = wav.readframes(wav.getnframes())

    if channels != 1:
        raise OpenVNError("8SVX conversion currently requires mono WAV")
    if sample_width != 2:
        raise OpenVNError("8SVX conversion currently requires 16-bit WAV")

    samples = struct.unpack("<" + "h" * (len(frames) // 2), frames)
    body = bytes(max(0, min(255, (sample >> 8) + 128)) for sample in samples)

    vhdr = struct.pack(">IIIHBBI", len(body), 0, 0, frame_rate, 1, 0, 0x10000)
    name = src.stem.encode("latin-1", errors="replace")
    chunks = []
    for chunk_id, payload in ((b"VHDR", vhdr), (b"NAME", name), (b"BODY", body)):
        padded = payload + (b"\0" if len(payload) % 2 else b"")
        chunks.append(chunk_id + struct.pack(">I", len(payload)) + padded)

    form_payload = b"8SVX" + b"".join(chunks)
    dst.write_bytes(b"FORM" + struct.pack(">I", len(form_payload)) + form_payload)
    return dst


def convert_asset(
    source: Path,
    destination_root: Path,
    *,
    relative: Path,
    profile: AmigaProfile,
    is_music: bool,
) -> tuple[Path, dict[str, str]]:
    suffix = source.suffix.lower()

    if profile.id == "amiga-rtg":
        destination = destination_root / relative
        destination.parent.mkdir(parents=True, exist_ok=True)
        destination.write_bytes(source.read_bytes())
        media_format = suffix.removeprefix(".")
        return destination, {"source": media_format, "target": media_format}

    if suffix == ".png":
        destination = (destination_root / relative).with_suffix(".iff")
        colors = int(profile.graphics.get("colors", 32))
        convert_png_to_ilbm(source, destination, colors=colors)
        return destination, {"source": "png", "target": "ilbm"}

    if suffix == ".wav" and not is_music:
        destination = (destination_root / relative).with_suffix(".8svx")
        convert_wav_to_8svx(source, destination)
        return destination, {"source": "wav", "target": "8svx"}

    if is_music:
        if suffix == ".mod":
            destination = destination_root / relative
            destination.parent.mkdir(parents=True, exist_ok=True)
            destination.write_bytes(source.read_bytes())
            return destination, {"source": "mod", "target": "mod"}
        raise OpenVNError(f"classic Amiga music must be supplied as MOD: {relative}")

    raise OpenVNError(f"unsupported Amiga asset format: {relative}")
