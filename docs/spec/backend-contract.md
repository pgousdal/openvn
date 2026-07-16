# Backend Contract

A backend receives a validated OpenVN Story Format 0.3 document.

A backend may generate source files, runtime data or a complete target project,
but it must not reinterpret Ink syntax.

A backend must:

- validate the supported OpenVN format version
- preserve node semantics
- preserve choice ordering
- preserve entry and graph targets
- fail clearly when a target platform cannot support required semantics

Initial planned backends:

- Ren'Py project generator
- Native Amiga runtime package using ARexx plus native C
