# Presentation Commands

OpenVN Story Format 0.4 adds presentation nodes.

Ink source syntax:

```text
#openvn scene lighthouse_storm
#openvn show erik neutral
#openvn hide erik
#openvn music storm
#openvn music stop
#openvn sound radio
```

Semantic nodes:

- `scene`
- `show`
- `hide`
- `music`
- `sound`

Ren'Py translations:

- `scene bg <background>`
- `show <character> <pose>`
- `hide <character>`
- `play music audio.<track>`
- `stop music`
- `play sound audio.<sound>`

The same semantic nodes will later map to ARexx commands and native AmigaOS
services. The native C layer remains system-friendly and uses AmigaOS APIs
rather than ACE.
