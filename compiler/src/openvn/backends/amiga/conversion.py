from __future__ import annotations

import struct
import wave
from pathlib import Path
from typing import Any

from PIL import Image

from ...errors import OpenVNError
from .profiles import AmigaProfile


def _positive_int(graphics: dict[str, Any], key: str) -> int:
    value = graphics.get(key)
    if not isinstance(value, int) or isinstance(value, bool) or value <= 0:
        raise OpenVNError(f"Amiga graphics profile field '{key}' must be a positive integer")
    return value


def _resampling_lanczos() -> Image.Resampling:
    return Image.Resampling.LANCZOS


def prepare_image_for_profile(source: str | Path, profile: AmigaProfile) -> Image.Image:
    """Return an RGB image with the exact geometry required by an Amiga profile.

    Classic full-screen backgrounds use a centre-cropped ``cover`` transform by
    default. This avoids letterboxing and guarantees that the runtime never has
    to scale or clip oversized source artwork.
    """

    width = _positive_int(profile.graphics, "width")
    height = _positive_int(profile.graphics, "height")
    fit = str(profile.graphics.get("fit", "cover"))

    with Image.open(source) as opened:
        image = opened.convert("RGB")

    if fit == "stretch":
        return image.resize((width, height), _resampling_lanczos())

    if fit != "cover":
        raise OpenVNError(f"unsupported Amiga graphics fit mode: {fit}")

    source_width, source_height = image.size
    scale = max(width / source_width, height / source_height)
    resized_width = max(width, round(source_width * scale))
    resized_height = max(height, round(source_height * scale))
    resized = image.resize((resized_width, resized_height), _resampling_lanczos())

    left = (resized_width - width) // 2
    top = (resized_height - height) // 2
    return resized.crop((left, top, left + width, top + height))


def convert_png_to_ilbm(
    source: str | Path,
    destination: str | Path,
    *,
    colors: int,
    width: int | None = None,
    height: int | None = None,
    fit: str = "cover",
) -> Path:
    src = Path(source)
    dst = Path(destination)
    dst.parent.mkdir(parents=True, exist_ok=True)

    with Image.open(src) as opened:
        image = opened.convert("RGB")

    if (width is None) != (height is None):
        raise OpenVNError("ILBM conversion requires both width and height")

    if width is not None and height is not None:
        if width <= 0 or height <= 0:
            raise OpenVNError("ILBM dimensions must be positive")
        if fit == "stretch":
            image = image.resize((width, height), _resampling_lanczos())
        elif fit == "cover":
            source_width, source_height = image.size
            scale = max(width / source_width, height / source_height)
            resized_width = max(width, round(source_width * scale))
            resized_height = max(height, round(source_height * scale))
            image = image.resize((resized_width, resized_height), _resampling_lanczos())
            left = (resized_width - width) // 2
            top = (resized_height - height) // 2
            image = image.crop((left, top, left + width, top + height))
        else:
            raise OpenVNError(f"unsupported Amiga graphics fit mode: {fit}")

    image = image.quantize(colors=colors, method=Image.Quantize.MEDIANCUT)
    image_width, image_height = image.size
    palette = image.getpalette()[: colors * 3]
    palette += [0] * (colors * 3 - len(palette))
    pixels = image.tobytes()

    bmhd = struct.pack(
        ">HHhhBBBBHBBhh",
        image_width,
        image_height,
        0,
        0,
        (colors - 1).bit_length(),
        0,
        0,
        0,
        0,
        10,
        11,
        image_width,
        image_height,
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
) -> tuple[Path, dict[str, str | int]]:
    suffix = source.suffix.lower()

    if profile.id == "amiga-rtg":
        destination = destination_root / relative
        destination.parent.mkdir(parents=True, exist_ok=True)
        destination.write_bytes(source.read_bytes())
        media_format = suffix.removeprefix(".")
        return destination, {"source": media_format, "target": media_format}

    if suffix == ".png":
        destination = (destination_root / relative).with_suffix(".iff")
        colors = _positive_int(profile.graphics, "colors")
        width = _positive_int(profile.graphics, "width")
        height = _positive_int(profile.graphics, "height")
        fit = str(profile.graphics.get("fit", "cover"))
        convert_png_to_ilbm(
            source,
            destination,
            colors=colors,
            width=width,
            height=height,
            fit=fit,
        )
        return destination, {
            "source": "png",
            "target": "ilbm",
            "width": width,
            "height": height,
            "colors": colors,
            "fit": fit,
        }

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
