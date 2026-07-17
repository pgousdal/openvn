# The Last Lighthouse

The Last Lighthouse is the official OpenVN reference demo.

It demonstrates:

- Story Format 0.4
- branching and five endings
- scenes and character poses
- music and sound effects
- asset manifest validation
- deterministic Ren'Py export

## Export

```bash
uv run --project compiler openvn export       examples/the-last-lighthouse       --backend renpy       --output build/the-last-lighthouse       --clean
```

Open `build/the-last-lighthouse` in the Ren'Py launcher.

The bundled artwork and audio are deliberately simple generated reference
assets. They are valid media files and make the exported project runnable,
while keeping the repository self-contained.
