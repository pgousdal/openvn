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

## Conditions and Branching

Conditions compare one named runtime variable with one literal of the same type.
No coercion is performed: a missing variable, type mismatch, or invalid operator
causes evaluation to fail.

Supported forms are:

- bool: direct `flag`, negated `not flag`, `==`, and `!=` with `true` or `false`;
- int32: `==`, `!=`, `<`, `<=`, `>`, and `>=` with a signed 32-bit literal;
- string: `==` and `!=` with a quoted UTF-8 string literal.

The evaluator API is declared in `openvn_condition.h`:

```c
int openvn_condition_evaluate(
    const OpenVNCondition *condition,
    int *result
);
```

It returns zero for invalid input, an unknown variable, a type mismatch, or an
operator that is not valid for the variable type. On failure, the output value
and variable store are unchanged.

Conditions use the existing OpenVN command syntax in Ink sources:

```ink
#openvn set_bool terminal_online true
#openvn set_int trust 2
#openvn set_string player_name "Alice"

#openvn if terminal_online
The terminal responds.
#openvn else
The terminal remains dark.
#openvn end

#openvn if trust >= 2
Alice decides to continue.
#openvn end
```

`else` is optional, and blocks may be nested. The compiler emits explicit true
and false node IDs and a deterministic jump around an else body. It reports
malformed blocks and literals with source locations.

Successful evaluation logs diagnostics such as:

```text
CONDITION trust >= 2
CONDITION result = true
BRANCH target = start-0011
```

Errors identify unknown variables, type mismatches, and invalid operators. The
current implementation intentionally has no arithmetic expressions, precedence,
implicit conversion, call/return, persistence, or general-purpose scripting.
