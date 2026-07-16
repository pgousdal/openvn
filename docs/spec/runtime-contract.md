# Runtime Contract

An OpenVN runtime consuming Story Format 0.3 must:

1. Begin execution at `entry`.
2. Display `TextNode.text` and continue through `TextNode.next`.
3. Present all `ChoiceNode.options` and follow the selected target.
4. Follow `JumpNode.target` without rendering content.
5. stop cleanly at `EndNode`.
6. Reject unknown node types and unsupported format versions.
7. Report malformed targets rather than continuing unpredictably.

## Amiga runtime

The Amiga implementation is intentionally independent of ACE.

Architecture:

- ARexx implements as much story and orchestration logic as practical.
- Native Amiga C provides only low-level services that require it:
  graphics, audio, input, filesystem access and memory handling.
- ACE is not a dependency and is not part of the required runtime contract.
