# Runtime API

## Runtime Variables

The Amiga-native runtime provides a deterministic, process-wide variable store for
booleans, signed 32-bit integers, and UTF-8 strings. Include
`openvn_variables.h` to use it.

```c
int openvn_set_bool(const char *name, int value);
int openvn_get_bool(const char *name, int *value);
int openvn_set_int(const char *name, int32_t value);
int openvn_get_int(const char *name, int32_t *value);
int openvn_set_string(const char *name, const char *value);
int openvn_get_string(const char *name, char *value, size_t value_size);
void openvn_variables_reset(void);
```

Setters create a named variable or overwrite its existing type and value. Getters
return zero for an unknown name, a type mismatch, a null output pointer, or a
string output buffer that is too small. Failed setters leave existing variables
unchanged. String limits count UTF-8 bytes excluding the terminating null byte.

Storage is static and uses no dynamic allocation. These compile-time definitions
configure it:

- `OPENVN_MAX_VARIABLES` (default `32`)
- `OPENVN_VARIABLE_NAME_MAX` (default `31` bytes)
- `OPENVN_VARIABLE_STRING_MAX` (default `127` bytes)

Call `openvn_variables_reset()` to clear all variables. Player initialization does
this automatically. Assignments log a diagnostic such as `VARIABLE score = 10`.

Ink sources create assignments with OpenVN commands:

```ink
#openvn set_bool has_key true
#openvn set_int score 10
#openvn set_string player_name Alice
```

The compiler emits `SET_BOOL`, `SET_INT`, and `SET_STRING` story nodes, which the
native player executes when reached.
