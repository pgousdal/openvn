from openvn.model import EndNode, JumpNode, Story, TextNode
from openvn.model.validation import validate_story


def test_story_ok():
    s = Story(
        nodes=[
            TextNode(id="start", type="text", text="Hi"),
            JumpNode(id="j1", type="jump", target="end"),
            EndNode(id="end", type="end"),
        ]
    )
    assert validate_story(s) == []
    assert s.to_dict()["format"] == "openvn-story"


def test_bad_jump():
    s = Story(nodes=[JumpNode(id="j", type="jump", target="missing")])
    assert validate_story(s)
