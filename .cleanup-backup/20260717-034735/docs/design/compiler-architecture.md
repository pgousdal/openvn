# Compiler Architecture

The M2 compiler performs four steps:

1. Load and validate `project.yaml`.
2. Load and minimally validate the Ink entry file.
3. Build an OpenVN story document.
4. Write `story.openvn.json`.

Runtime backends are deliberately excluded from M2.

The generated JSON currently preserves the Ink source text. A later milestone
will introduce a real semantic story model and an Ink compiler integration.
