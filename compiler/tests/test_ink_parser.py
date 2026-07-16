from pathlib import Path

import pytest

from openvn.errors import SourceError
from openvn.ink_parser import parse_ink_file, parse_ink_text
from openvn.model import ChoiceNode, EndNode, JumpNode, TextNode
from openvn.model.validation import validate_story


def test_parse_text_links_to_end() -> None:
    story = parse_ink_text("Hello.\n-> END\n")
    text = story.nodes[0]
    assert isinstance(text, TextNode)
    assert text.next == "start-0002"
    assert isinstance(story.nodes[1], EndNode)
    assert story.entry == "start-0001"
    assert story.symbols == {"start": "start-0001"}


def test_knot_targets_resolve_to_node_ids() -> None:
    story = parse_ink_text(
        """
        === start ===
        -> next
        === next ===
        Done.
        -> END
        """
    )
    jump = story.nodes[0]
    assert isinstance(jump, JumpNode)
    assert jump.target == "next-0001"
    assert story.symbols["next"] == "next-0001"
    assert validate_story(story) == []


def test_choice_targets_resolve_to_node_ids() -> None:
    story = parse_ink_text(
        """
        === start ===
        * [Left] -> left
        * [Right] -> right
        === left ===
        Left.
        -> END
        === right ===
        Right.
        -> END
        """
    )
    choice = story.nodes[0]
    assert isinstance(choice, ChoiceNode)
    assert [option.target for option in choice.options] == ["left-0001", "right-0001"]
    assert validate_story(story) == []


def test_direct_node_target_is_preserved() -> None:
    story = parse_ink_text("-> start-0002\n-> END\n")
    jump = story.nodes[0]
    assert isinstance(jump, JumpNode)
    assert jump.target == "start-0002"


def test_unknown_target_is_left_for_validation() -> None:
    story = parse_ink_text("-> missing\n")
    assert "unknown jump target: missing" in validate_story(story)


def test_duplicate_knot_fails() -> None:
    with pytest.raises(SourceError, match="duplicate knot"):
        parse_ink_text(
            """
            === one ===
            First.
            === one ===
            Second.
            """
        )


def test_empty_knot_name_fails() -> None:
    with pytest.raises(SourceError, match="empty knot name"):
        parse_ink_text("======\n")


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
