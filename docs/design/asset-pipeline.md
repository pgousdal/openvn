# Asset Pipeline Foundation

M4 PR3 introduces project-local asset manifests.

Manifest location:

```text
assets/manifest.yaml
```

Supported categories:

- backgrounds
- character poses
- music
- sound effects

The Ren'Py backend generates:

- `game/images.rpy`
- `game/audio.rpy`

and copies referenced files into the generated game directory.
