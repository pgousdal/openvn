# OpenVN Diagnostics

- `OVN001`: source or Ink syntax error
- `OVN002`: unresolved graph target
- `OVN003`: duplicate knot
- `OVN004`: unreachable node warning
- `OVN005`: invalid OpenVN story structure or schema

Diagnostics may contain:

- severity
- code
- message
- file
- line
- column

Warnings do not block normal compilation. Strict mode treats warnings as
blocking.
