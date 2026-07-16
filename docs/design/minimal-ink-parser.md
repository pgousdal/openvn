# Minimal Ink Parser

M3 PR2 introduces a deliberately small Ink subset.

Supported:

- plain text
- `=== knot ===`
- `* [Choice] -> target`
- `-> target`
- `-> END`
- blank lines and `//` comments

Not supported yet:

- variables
- conditions
- stitches
- tunnels
- functions
- lists
- tags

The parser emits the semantic story model from M3 PR1.
