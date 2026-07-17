from pathlib import Path

from PIL import Image

REPOSITORY = Path(__file__).parents[2]


def test_demo_background_has_no_embedded_title_plate() -> None:
    image = Image.open(
        REPOSITORY / "examples/demo/assets/backgrounds/room.png"
    ).convert("RGB")
    region = image.crop((18, 18, 251, 54))
    dark_pixels = sum(
        1 for r, g, b in region.getdata() if r < 16 and g < 16 and b < 16
    )
    assert dark_pixels < 8


def test_demo_character_has_no_embedded_name_plate() -> None:
    image = Image.open(
        REPOSITORY / "examples/demo/assets/characters/alice_neutral.png"
    ).convert("RGBA")
    region = image.crop((48, 258, 190, 290))
    black_opaque = sum(
        1
        for r, g, b, a in region.getdata()
        if a > 0 and r < 32 and g < 32 and b < 32
    )
    assert black_opaque < 16
