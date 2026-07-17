from __future__ import annotations

import struct
import wave
from collections.abc import Iterable
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
    """Return an RGB image with the exact geometry required by an Amiga profile."""

    width = _positive_int(profile.graphics, "width")
    height = _positive_int(profile.graphics, "height")
    fit = str(profile.graphics.get("fit", "cover"))

    with Image.open(source) as opened:
        image = opened.convert("RGB")

    return _fit_background(image, width=width, height=height, fit=fit)


def _fit_background(image: Image.Image, *, width: int, height: int, fit: str) -> Image.Image:
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


def _fit_character(
    image: Image.Image,
    *,
    max_width: int,
    max_height: int,
) -> tuple[Image.Image, tuple[int, int, int, int]]:
    rgba = image.convert("RGBA")
    alpha = rgba.getchannel("A")
    bbox = alpha.getbbox()
    if bbox is None:
        raise OpenVNError("character image contains no visible pixels")

    cropped = rgba.crop(bbox)
    width, height = cropped.size
    scale = min(1.0, max_width / width, max_height / height)
    if scale < 1.0:
        resized = (
            max(1, round(width * scale)),
            max(1, round(height * scale)),
        )
        cropped = cropped.resize(resized, _resampling_lanczos())

    return cropped, bbox


def _palette_image(palette: bytes, colors: int) -> Image.Image:
    if len(palette) != colors * 3:
        raise OpenVNError("Amiga palette has an unexpected size")

    image = Image.new("P", (1, 1))
    padded = list(palette) + [0] * (768 - len(palette))
    image.putpalette(padded)
    return image


def build_shared_palette(
    sources: Iterable[Path],
    *,
    colors: int,
) -> bytes:
    """Build one deterministic palette shared by backgrounds and characters.

    Index zero is reserved for transparent character pixels. The remaining
    entries are learned from all opaque source artwork in the package.
    """

    if colors < 2 or colors > 256:
        raise OpenVNError("shared Amiga palette must contain between 2 and 256 colors")

    samples: list[Image.Image] = []
    for source in sorted((Path(path) for path in sources), key=lambda path: str(path)):
        with Image.open(source) as opened:
            rgba = opened.convert("RGBA")

        alpha = rgba.getchannel("A")
        opaque = Image.new("RGB", rgba.size, (0, 0, 0))
        opaque.paste(rgba.convert("RGB"), mask=alpha)
        bbox = alpha.getbbox()
        if bbox is not None:
            samples.append(opaque.crop(bbox))

    if not samples:
        return bytes(colors * 3)

    width = max(image.width for image in samples)
    height = sum(image.height for image in samples)
    atlas = Image.new("RGB", (width, height), (0, 0, 0))
    y = 0
    for image in samples:
        atlas.paste(image, (0, y))
        y += image.height

    quantized = atlas.quantize(colors=colors - 1, method=Image.Quantize.MEDIANCUT)
    learned = quantized.getpalette()[: (colors - 1) * 3]
    learned += [0] * ((colors - 1) * 3 - len(learned))
    return bytes([0, 0, 0, *learned])


def _quantize_background(image: Image.Image, *, palette: bytes, colors: int) -> Image.Image:
    palette_image = _palette_image(palette, colors)
    return image.convert("RGB").quantize(palette=palette_image, dither=Image.Dither.NONE)


def _quantize_character(image: Image.Image, *, palette: bytes, colors: int) -> Image.Image:
    rgba = image.convert("RGBA")
    alpha = rgba.getchannel("A")

    # Quantize against entries 1..N. A temporary palette places those colors at
    # 0..N-2, after which the resulting indices are shifted by one.
    opaque_palette = palette[3:]
    palette_image = _palette_image(opaque_palette, colors - 1)
    quantized = rgba.convert("RGB").quantize(
        palette=palette_image,
        dither=Image.Dither.NONE,
    )

    source_indices = quantized.tobytes()
    alpha_bytes = alpha.tobytes()
    output = bytes(
        0 if alpha_value < 128 else min(colors - 1, index + 1)
        for index, alpha_value in zip(source_indices, alpha_bytes, strict=True)
    )

    result = Image.frombytes("P", rgba.size, output)
    result.putpalette(list(palette) + [0] * (768 - len(palette)))
    return result


def _write_ilbm(
    image: Image.Image,
    destination: str | Path,
    *,
    colors: int,
    transparent: bool,
) -> Path:
    dst = Path(destination)
    dst.parent.mkdir(parents=True, exist_ok=True)

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
        2 if transparent else 0,
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


def convert_png_to_ilbm(
    source: str | Path,
    destination: str | Path,
    *,
    colors: int,
    width: int | None = None,
    height: int | None = None,
    fit: str = "cover",
    palette: bytes | None = None,
) -> Path:
    src = Path(source)

    with Image.open(src) as opened:
        image = opened.convert("RGB")

    if (width is None) != (height is None):
        raise OpenVNError("ILBM conversion requires both width and height")

    if width is not None and height is not None:
        if width <= 0 or height <= 0:
            raise OpenVNError("ILBM dimensions must be positive")
        image = _fit_background(image, width=width, height=height, fit=fit)

    if palette is None:
        quantized = image.quantize(colors=colors, method=Image.Quantize.MEDIANCUT)
    else:
        quantized = _quantize_background(image, palette=palette, colors=colors)

    return _write_ilbm(quantized, destination, colors=colors, transparent=False)


def convert_character_png_to_ilbm(
    source: str | Path,
    destination: str | Path,
    *,
    colors: int,
    palette: bytes,
    max_width: int,
    max_height: int,
) -> tuple[Path, dict[str, int | str | bool]]:
    with Image.open(source) as opened:
        character, source_bbox = _fit_character(
            opened,
            max_width=max_width,
            max_height=max_height,
        )

    quantized = _quantize_character(character, palette=palette, colors=colors)
    output = _write_ilbm(quantized, destination, colors=colors, transparent=True)
    width, height = quantized.size
    return output, {
        "source": "png",
        "target": "ilbm",
        "kind": "character",
        "width": width,
        "height": height,
        "colors": colors,
        "transparent_index": 0,
        "mask": True,
        "origin_x": width // 2,
        "origin_y": height,
        "source_crop_x": source_bbox[0],
        "source_crop_y": source_bbox[1],
        "source_crop_width": source_bbox[2] - source_bbox[0],
        "source_crop_height": source_bbox[3] - source_bbox[1],
    }


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
    shared_palette: bytes | None = None,
) -> tuple[Path, dict[str, str | int | bool]]:
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

        if relative.parts and relative.parts[0] == "characters":
            if shared_palette is None:
                shared_palette = build_shared_palette([source], colors=colors)
            max_width = int(profile.graphics.get("character_max_width", width))
            max_height = int(profile.graphics.get("character_max_height", height))
            return convert_character_png_to_ilbm(
                source,
                destination,
                colors=colors,
                palette=shared_palette,
                max_width=max_width,
                max_height=max_height,
            )

        fit = str(profile.graphics.get("fit", "cover"))
        convert_png_to_ilbm(
            source,
            destination,
            colors=colors,
            width=width,
            height=height,
            fit=fit,
            palette=shared_palette,
        )
        metadata: dict[str, str | int | bool] = {
            "source": "png",
            "target": "ilbm",
            "width": width,
            "height": height,
            "colors": colors,
            "fit": fit,
        }
        if shared_palette is not None:
            metadata["kind"] = "background"
            metadata["shared_palette"] = True
        return destination, metadata

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
