# Native Amiga Runtime Direction

The OpenVN Amiga runtime is ARexx-first and does not depend on ACE.

The native C component must also be system-friendly. It should use
AmigaOS facilities directly instead of bypassing the operating system or
recreating an unrelated engine abstraction.

Preferred services include:

- `exec.library` for tasks, messages and memory
- `dos.library` for files and processes
- `intuition.library` for windows and user interaction
- `graphics.library` and `layers.library` for display operations
- `asl.library`, `iffparse.library` and `datatypes.library` where useful
- devices such as `timer.device`, `audio.device` and `input.device`

ARexx owns practical story orchestration. Native C supplies the small set of
low-level services that require performance, hardware access or direct OS
integration.
