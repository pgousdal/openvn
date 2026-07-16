from openvn.backends.renpy.generator import render_script


def test_render_presentation_nodes() -> None:
    document = {
        "format": "openvn-story",
        "version": "0.4",
        "entry": "start-0001",
        "symbols": {"start": "start-0001"},
        "nodes": [
            {
                "id": "start-0001",
                "type": "scene",
                "background": "room",
                "next": "start-0002",
            },
            {
                "id": "start-0002",
                "type": "show",
                "character": "alice",
                "pose": "happy",
                "next": "start-0003",
            },
            {
                "id": "start-0003",
                "type": "music",
                "track": "intro",
                "next": "start-0004",
            },
            {
                "id": "start-0004",
                "type": "sound",
                "sound": "click",
                "next": "start-0005",
            },
            {
                "id": "start-0005",
                "type": "hide",
                "character": "alice",
                "next": "start-0006",
            },
            {
                "id": "start-0006",
                "type": "music",
                "track": None,
                "next": "start-0007",
            },
            {"id": "start-0007", "type": "end"},
        ],
    }

    script = render_script(document)
    assert "scene bg room" in script
    assert "show alice happy" in script
    assert "play music audio.intro" in script
    assert "play sound audio.click" in script
    assert "hide alice" in script
    assert "stop music" in script
