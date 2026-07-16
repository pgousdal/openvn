# Compiler Integration

M3 PR3 connects the minimal Ink parser to the OpenVN compiler.

The compiler pipeline is now:

```text
project.yaml
main.ink
   ↓
project loader
   ↓
minimal Ink parser
   ↓
semantic Story model
   ↓
semantic validation
   ↓
story.openvn.json
```

The generated file no longer embeds raw Ink source. Runtime backends consume
semantic nodes only.

Format version 0.2 supports:

- text nodes
- choice nodes
- jump nodes
- end nodes
