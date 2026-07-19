#include "openvn_variables.h"

#include <stdio.h>
#include <string.h>

typedef enum OpenVNVariableType {
    OPENVN_VARIABLE_UNUSED = 0,
    OPENVN_VARIABLE_BOOL,
    OPENVN_VARIABLE_INT,
    OPENVN_VARIABLE_STRING
} OpenVNVariableType;

typedef struct OpenVNVariable {
    char name[OPENVN_VARIABLE_NAME_MAX + 1];
    OpenVNVariableType type;
    union {
        int boolean;
        int32_t integer;
        char string[OPENVN_VARIABLE_STRING_MAX + 1];
    } value;
} OpenVNVariable;

static OpenVNVariable variables[OPENVN_MAX_VARIABLES];
static size_t variable_count;

static int valid_name(const char *name) {
    size_t length;

    if (name == 0 || name[0] == '\0') return 0;
    length = strlen(name);
    return length <= OPENVN_VARIABLE_NAME_MAX;
}

static int find_variable(const char *name) {
    size_t index;

    if (!valid_name(name)) return -1;
    for (index = 0U; index < variable_count; index++) {
        if (strcmp(variables[index].name, name) == 0) return (int)index;
    }
    return -1;
}

static OpenVNVariable *variable_for_set(const char *name) {
    int index;
    OpenVNVariable *variable;

    index = find_variable(name);
    if (index >= 0) return &variables[index];
    if (!valid_name(name) || variable_count >= OPENVN_MAX_VARIABLES) return 0;

    variable = &variables[variable_count++];
    strcpy(variable->name, name);
    return variable;
}

void openvn_variables_reset(void) {
    memset(variables, 0, sizeof(variables));
    variable_count = 0U;
}

int openvn_set_bool(const char *name, int value) {
    OpenVNVariable *variable;

    variable = variable_for_set(name);
    if (variable == 0) return 0;
    variable->type = OPENVN_VARIABLE_BOOL;
    variable->value.boolean = value != 0;
    printf("VARIABLE %s = %s\n", name, variable->value.boolean ? "true" : "false");
    return 1;
}

int openvn_get_bool(const char *name, int *value) {
    int index;

    index = find_variable(name);
    if (index < 0 || value == 0 ||
        variables[index].type != OPENVN_VARIABLE_BOOL) {
        return 0;
    }
    *value = variables[index].value.boolean;
    return 1;
}

int openvn_set_int(const char *name, int32_t value) {
    OpenVNVariable *variable;

    variable = variable_for_set(name);
    if (variable == 0) return 0;
    variable->type = OPENVN_VARIABLE_INT;
    variable->value.integer = value;
    printf("VARIABLE %s = %ld\n", name, (long)value);
    return 1;
}

int openvn_get_int(const char *name, int32_t *value) {
    int index;

    index = find_variable(name);
    if (index < 0 || value == 0 ||
        variables[index].type != OPENVN_VARIABLE_INT) {
        return 0;
    }
    *value = variables[index].value.integer;
    return 1;
}

int openvn_set_string(const char *name, const char *value) {
    OpenVNVariable *variable;

    if (value == 0 || strlen(value) > OPENVN_VARIABLE_STRING_MAX) return 0;
    variable = variable_for_set(name);
    if (variable == 0) return 0;
    variable->type = OPENVN_VARIABLE_STRING;
    strcpy(variable->value.string, value);
    printf("VARIABLE %s = %s\n", name, value);
    return 1;
}

int openvn_get_string(const char *name, char *value, size_t value_size) {
    int index;
    size_t length;

    index = find_variable(name);
    if (index < 0 || value == 0 ||
        variables[index].type != OPENVN_VARIABLE_STRING) {
        return 0;
    }
    length = strlen(variables[index].value.string);
    if (value_size <= length) return 0;
    strcpy(value, variables[index].value.string);
    return 1;
}
