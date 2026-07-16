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


def test_unknown_target_is_reported() -> None:
    story = parse_ink_text("-> missing\n")
    diagnostics = validate_story(story)
    assert diagnostics[0].code == "OVN002"
    assert diagnostics[0].message == "unknown jump target: missing"


def test_duplicate_knot_has_diagnostic() -> None:
    with pytest.raises(SourceError) as captured:
        parse_ink_text(
            """
            === one ===
            First.
            === one ===
            Second.
            """,
            source_path="main.ink",
        )
    assert captured.value.diagnostic is not None
    assert captured.value.diagnostic.code == "OVN003"
    assert captured.value.diagnostic.line == 4


def test_choice_error_has_location() -> None:
    with pytest.raises(SourceError) as captured:
        parse_ink_text("* [Continue]\n", source_path="main.ink")
    assert captured.value.diagnostic is not None
    assert captured.value.diagnostic.code == "OVN001"
    assert captured.value.diagnostic.file == "main.ink"
    assert captured.value.diagnostic.line == 1
    assert captured.value.diagnostic.column == 1


def test_jump_error_has_location() -> None:
    with pytest.raises(SourceError) as captured:
        parse_ink_text("->\n", source_path="main.ink")
    assert captured.value.diagnostic is not None
    assert captured.value.diagnostic.line == 1


def test_empty_source_fails() -> None:
    with pytest.raises(SourceError, match="no story nodes"):
        parse_ink_text("\n// only comment\n")


def test_parse_file_uses_source_path(tmp_path: Path) -> None:
    path = tmp_path / "main.ink"
    path.write_text("* [Broken]\n", encoding="utf-8")
    with pytest.raises(SourceError) as captured:
        parse_ink_file(path)
    assert captured.value.diagnostic is not None
    assert captured.value.diagnostic.file == str(path)
