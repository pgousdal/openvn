from pathlib import Path

from PIL import Image

REPOSITORY = Path(__file__).parents[2]


def _pixels(image: Image.Image):
    return image.get_flattened_data()


def test_demo_background_has_no_embedded_title_plate() -> None:
    image = Image.open(
        REPOSITORY / "examples/demo/assets/backgrounds/room.png"
    ).convert("RGB")
    region = image.crop((18, 18, 251, 54))
    dark_pixels = sum(
        1 for r, g, b in _pixels(region) if r < 16 and g < 16 and b < 16
    )
    assert dark_pixels < 8


def test_demo_character_has_no_embedded_name_ui() -> None:
    image = Image.open(
        REPOSITORY / "examples/demo/assets/characters/alice_neutral.png"
    ).convert("RGBA")
    region = image.crop((48, 258, 191, 291))

    black_opaque = sum(
        1
        for r, g, b, a in _pixels(region)
        if a > 200 and r < 32 and g < 32 and b < 32
    )
    light_text = sum(
        1
        for r, g, b, a in _pixels(region)
        if a > 200 and r > 185 and g > 185 and b > 160
    )
    opaque_colours = {
        (r, g, b)
        for r, g, b, a in _pixels(region)
        if a > 200
    }

    assert black_opaque == 0
    assert light_text == 0
    assert len(opaque_colours) == 1
