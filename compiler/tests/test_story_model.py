from openvn.model import ChoiceNode, ChoiceOption, EndNode, JumpNode, Story, TextNode
from openvn.model.validation import (
    reachable_node_ids,
    unreachable_diagnostics,
    unreachable_node_ids,
    validate_story,
)


def test_story_serialization() -> None:
    story = Story(
        entry="start-0001",
        symbols={"start": "start-0001"},
        nodes=[
            TextNode(id="start-0001", type="text", text="Hi", next="start-0002"),
            EndNode(id="start-0002", type="end"),
        ],
    )

    data = story.to_dict()
    assert data["format"] == "openvn-story"
    assert data["version"] == "0.3"
    assert data["entry"] == "start-0001"
    assert data["symbols"] == {"start": "start-0001"}


def test_validation_accepts_valid_graph() -> None:
    story = Story(
        entry="start",
        nodes=[
            TextNode(id="start", type="text", text="Hi", next="jump"),
            JumpNode(id="jump", type="jump", target="end"),
            EndNode(id="end", type="end"),
        ],
    )
    assert validate_story(story) == []


def test_validation_reports_duplicate_ids() -> None:
    story = Story(
        entry="same",
        nodes=[
            TextNode(id="same", type="text", text="A"),
            EndNode(id="same", type="end"),
        ],
    )
    diagnostics = validate_story(story)
    assert diagnostics[0].code == "OVN005"
    assert diagnostics[0].message == "duplicate node id: same"


def test_validation_reports_invalid_entry_and_symbol() -> None:
    story = Story(entry="missing", symbols={"intro": "also-missing"})
    messages = [diagnostic.message for diagnostic in validate_story(story)]
    assert "unknown story entry: missing" in messages
    assert "symbol 'intro' has unknown target: also-missing" in messages


def test_validation_reports_all_edge_types() -> None:
    story = Story(
        entry="text",
        nodes=[
            TextNode(id="text", type="text", text="A", next="missing-next"),
            JumpNode(id="jump", type="jump", target="missing-jump"),
            ChoiceNode(
                id="choice",
                type="choice",
                options=[ChoiceOption(text="Go", target="missing-choice")],
            ),
        ],
    )
    diagnostics = validate_story(story)
    messages = [diagnostic.message for diagnostic in diagnostics]
    assert "node 'text' has unknown next target: missing-next" in messages
    assert "unknown jump target: missing-jump" in messages
    assert "unknown choice target: missing-choice" in messages
    assert all(diagnostic.code == "OVN002" for diagnostic in diagnostics)


def test_reachability() -> None:
    story = Story(
        entry="start",
        nodes=[
            TextNode(id="start", type="text", text="A", next="choice"),
            ChoiceNode(
                id="choice",
                type="choice",
                options=[
                    ChoiceOption(text="End", target="end"),
                    ChoiceOption(text="Other", target="other"),
                ],
            ),
            EndNode(id="end", type="end"),
            EndNode(id="other", type="end"),
            EndNode(id="unused", type="end"),
        ],
    )
    assert reachable_node_ids(story) == {"start", "choice", "end", "other"}
    assert unreachable_node_ids(story) == {"unused"}


def test_unreachable_diagnostic() -> None:
    story = Story(
        entry="start",
        nodes=[
            EndNode(id="start", type="end"),
            EndNode(id="unused", type="end"),
        ],
    )
    diagnostics = unreachable_diagnostics(story)
    assert len(diagnostics) == 1
    assert diagnostics[0].severity == "warning"
    assert diagnostics[0].code == "OVN004"
