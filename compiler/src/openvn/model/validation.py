from .nodes import ChoiceNode, JumpNode
from .story import Story


def validate_story(story: Story) -> list[str]:
    errors = []
    ids = {n.id for n in story.nodes}
    if len(ids) != len(story.nodes):
        errors.append("duplicate node id")
    for n in story.nodes:
        if isinstance(n, JumpNode) and n.target not in ids:
            errors.append(f"unknown jump target: {n.target}")
        if isinstance(n, ChoiceNode):
            for o in n.options:
                if o.target not in ids:
                    errors.append(f"unknown choice target: {o.target}")
    return errors
