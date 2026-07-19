import pytest

from openvn.errors import SourceError
from openvn.ink_parser import parse_ink_text
from openvn.model import ConditionNode, JumpNode, TextNode
from openvn.model.validation import validate_story


@pytest.mark.parametrize(
    ("expression", "value_type", "operator", "value"),
    [
        ("flag", "bool", "bool_true", None),
        ("not flag", "bool", "bool_false", None),
        ("flag == true", "bool", "==", True),
        ("flag != false", "bool", "!=", False),
        ("score == 10", "int", "==", 10),
        ("score != 10", "int", "!=", 10),
        ("score < 10", "int", "<", 10),
        ("score <= 10", "int", "<=", 10),
        ("score > 10", "int", ">", 10),
        ("score >= 10", "int", ">=", 10),
        ('player_name == "Alice"', "string", "==", "Alice"),
        ('player_name != "Alice"', "string", "!=", "Alice"),
    ],
)
def test_parse_supported_conditions(
    expression: str,
    value_type: str,
    operator: str,
    value: object,
) -> None:
    story = parse_ink_text(f"#openvn if {expression}\nTrue.\n#openvn end\n-> END\n")
    node = story.nodes[0]

    assert isinstance(node, ConditionNode)
    assert node.condition.value_type == value_type
    assert node.condition.operator == operator
    if value_type == "bool" and value is not None:
        assert node.condition.bool_value is value
    elif value_type == "int":
        assert node.condition.int_value == value
    elif value_type == "string":
        assert node.condition.string_value == value


def test_if_else_targets_are_resolved() -> None:
    story = parse_ink_text("#openvn if flag\nTrue.\n#openvn else\nFalse.\n#openvn end\nAfter.\n")
    condition = story.nodes[0]
    true_text = story.nodes[1]
    skip_else = story.nodes[2]
    false_text = story.nodes[3]
    after = story.nodes[4]

    assert isinstance(condition, ConditionNode)
    assert isinstance(true_text, TextNode)
    assert isinstance(skip_else, JumpNode)
    assert condition.true_target == true_text.id
    assert condition.false_target == false_text.id
    assert skip_else.target == after.id
    assert true_text.next == skip_else.id
    assert false_text.next == after.id
    assert validate_story(story) == []


def test_if_without_else_uses_continuation_as_false_target() -> None:
    story = parse_ink_text("#openvn if flag\nTrue.\n#openvn end\nAfter.\n")
    condition = story.nodes[0]
    assert isinstance(condition, ConditionNode)
    assert condition.true_target == story.nodes[1].id
    assert condition.false_target == story.nodes[2].id


def test_nested_conditions_resolve_independently() -> None:
    story = parse_ink_text(
        "#openvn if outer\n"
        "#openvn if inner\n"
        "Both.\n"
        "#openvn else\n"
        "Outer only.\n"
        "#openvn end\n"
        "#openvn else\n"
        "Neither.\n"
        "#openvn end\n"
        "After.\n"
    )
    conditions = [node for node in story.nodes if isinstance(node, ConditionNode)]
    assert len(conditions) == 2
    assert all(node.true_target and node.false_target for node in conditions)
    assert validate_story(story) == []


@pytest.mark.parametrize(
    ("source", "message"),
    [
        ("#openvn if\n", "missing variable name"),
        ("#openvn if score ==\n#openvn end\n", "missing condition literal"),
        ("#openvn if score <> 1\n#openvn end\n", "unsupported condition operator"),
        ("#openvn if score == ten\n#openvn end\n", "invalid integer literal"),
        ('#openvn if name == "Alice\n#openvn end\n', "unterminated condition string"),
        ('#openvn if name < "Alice"\n#openvn end\n', "invalid for string"),
        ("#openvn else\n", "else outside"),
        ("#openvn end\n", "end outside"),
        ("#openvn if flag\nText.\n", "missing end"),
        (
            "#openvn if flag\n#openvn else\n#openvn else\n#openvn end\n",
            "duplicate else",
        ),
    ],
)
def test_condition_diagnostics(source: str, message: str) -> None:
    with pytest.raises(SourceError, match=message) as captured:
        parse_ink_text(source, source_path="main.ink")
    assert captured.value.diagnostic is not None
    assert captured.value.diagnostic.file == "main.ink"
    assert captured.value.diagnostic.line is not None


def test_known_assignment_type_mismatch_is_rejected() -> None:
    with pytest.raises(SourceError, match="type mismatch"):
        parse_ink_text(
            '#openvn set_string score "10"\n#openvn if score >= 10\nText.\n#openvn end\n'
        )
