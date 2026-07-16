# Amiga Runtime Principle

The OpenVN Amiga runtime is ARexx-first.

ARexx should implement all practical story and gameplay logic, including:

- branching
- variables
- choices
- scene control
- save/load orchestration

Plain native C is reserved for low-level services that require it:

- graphics
- audio
- input
- file access
- memory management

ACE is not a required dependency.
