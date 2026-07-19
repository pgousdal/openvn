from openvn.ink_parser import parse_ink_text
from openvn.model import (
    HideNode,
    MusicNode,
    SceneNode,
    SetBoolNode,
    SetIntNode,
    SetStringNode,
    ShowNode,
    SoundNode,
)


def test_parse_presentation_commands() -> None:
    story = parse_ink_text(
        """
        #openvn scene lighthouse_storm
        #openvn show erik neutral
        #openvn sound radio
        #openvn music storm
        #openvn hide erik
        #openvn music stop
        -> END
        """
    )

    assert isinstance(story.nodes[0], SceneNode)
    assert isinstance(story.nodes[1], ShowNode)
    assert isinstance(story.nodes[2], SoundNode)
    assert isinstance(story.nodes[3], MusicNode)
    assert isinstance(story.nodes[4], HideNode)
    assert isinstance(story.nodes[5], MusicNode)
    assert story.nodes[5].track is None


def test_presentation_nodes_are_linked() -> None:
    story = parse_ink_text("#openvn scene room\nHello\n-> END\n")
    assert story.nodes[0].next == story.nodes[1].id


def test_parse_runtime_variable_commands() -> None:
    story = parse_ink_text(
        "#openvn set_bool has_key true\n"
        "#openvn set_int score -10\n"
        "#openvn set_string player_name Alice Smith\n"
        "-> END\n"
    )

    assert isinstance(story.nodes[0], SetBoolNode)
    assert story.nodes[0].value is True
    assert isinstance(story.nodes[1], SetIntNode)
    assert story.nodes[1].value == -10
    assert isinstance(story.nodes[2], SetStringNode)
    assert story.nodes[2].value == "Alice Smith"
