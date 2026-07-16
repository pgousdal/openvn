# Story Graph

M3 PR4 makes the OpenVN intermediate format an explicit directed graph.

## Graph fields

- `entry`: node where execution begins
- `symbols`: mapping from Ink knot names to concrete node IDs
- `nodes`: runtime-independent semantic nodes

## Edges

- `TextNode.next`
- `ChoiceOption.target`
- `JumpNode.target`
- `EndNode` has no outgoing edge

The compiler resolves Ink knot references during compilation. Runtime backends
therefore do not need to understand Ink knot names.

The validation layer checks entries, symbols, duplicate IDs and all edge
targets. Reachability helpers identify nodes that cannot be reached from the
story entry.
