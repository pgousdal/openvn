import wave
from pathlib import Path

from PIL import Image

from openvn.backends.amiga.conversion import (
    convert_png_to_ilbm,
    convert_wav_to_8svx,
)


def test_png_to_ilbm(tmp_path: Path) -> None:
    source = tmp_path / "image.png"
    Image.new("RGB", (16, 16), (10, 20, 30)).save(source)

    destination = convert_png_to_ilbm(
        source,
        tmp_path / "image.iff",
        colors=32,
    )
    data = destination.read_bytes()
    assert data.startswith(b"FORM")
    assert data[8:12] == b"ILBM"


def test_wav_to_8svx(tmp_path: Path) -> None:
    source = tmp_path / "sound.wav"
    with wave.open(str(source), "wb") as wav:
        wav.setnchannels(1)
        wav.setsampwidth(2)
        wav.setframerate(8000)
        wav.writeframes(b"\0\0" * 64)

    destination = convert_wav_to_8svx(source, tmp_path / "sound.8svx")
    data = destination.read_bytes()
    assert data.startswith(b"FORM")
    assert data[8:12] == b"8SVX"
