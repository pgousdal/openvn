from pathlib import Path

import pytest

from openvn.errors import SourceError
from openvn.ink_parser import parse_ink_file, parse_ink_text
from openvn.model import ChoiceNode, EndNode, JumpNode, TextNode


def test_parse_text_and_end() -> None:
    story = parse_ink_text("Hello.\n-> END\n")
    assert isinstance(story.nodes[0], TextNode)
    assert story.nodes[0].text == "Hello."
    assert isinstance(story.nodes[1], EndNode)


def test_parse_jump() -> None:
    story = parse_ink_text("-> next\n")
    node = story.nodes[0]
    assert isinstance(node, JumpNode)
    assert node.target == "next"


def test_parse_choices() -> None:
    story = parse_ink_text("* [Left] -> left\n* [Right] -> right\n")
    node = story.nodes[0]
    assert isinstance(node, ChoiceNode)
    assert [option.text for option in node.options] == ["Left", "Right"]
    assert [option.target for option in node.options] == ["left", "right"]


def test_parse_knot_changes_node_prefix() -> None:
    story = parse_ink_text("=== intro ===\nHello\n-> END\n")
    assert story.nodes[0].id.startswith("intro-")


def test_ignore_blank_lines_and_comments() -> None:
    story = parse_ink_text("\n// comment\nHello\n-> END\n")
    assert len(story.nodes) == 2


def test_empty_source_fails() -> None:
    with pytest.raises(SourceError, match="no story nodes"):
        parse_ink_text("\n// only comment\n")


def test_choice_requires_target() -> None:
    with pytest.raises(SourceError, match="choice must use"):
        parse_ink_text("* [Continue]\n")


def test_jump_requires_target() -> None:
    with pytest.raises(SourceError, match="missing jump target"):
        parse_ink_text("->\n")


def test_parse_file(tmp_path: Path) -> None:
    path = tmp_path / "main.ink"
    path.write_text("Hello\n-> END\n", encoding="utf-8")
    assert len(parse_ink_file(path).nodes) == 2
