# Ren'Py Backend

M4 PR1 introduces the first OpenVN backend.

Command:

```bash
openvn export examples/minimal --backend renpy --output build/renpy-minimal
```

The backend:

1. compiles the Ink project in strict mode
2. reads the OpenVN Story Format 0.3 document
3. generates a Ren'Py project
4. writes `game/script.rpy` and `game/options.rpy`

Supported semantic nodes:

- text
- choice
- jump
- end

The backend does not parse Ink and depends only on the stable OpenVN story
contract.
