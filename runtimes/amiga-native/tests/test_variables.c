#include "openvn_variables.h"

#include <assert.h>
#include <stdio.h>
#include <string.h>

int main(void) {
    char name[OPENVN_VARIABLE_NAME_MAX + 1];
    char string_value[OPENVN_VARIABLE_STRING_MAX + 1];
    char too_long[OPENVN_VARIABLE_STRING_MAX + 2];
    int bool_value;
    int32_t int_value;
    size_t index;

    openvn_variables_reset();
    assert(!openvn_get_int("unknown", &int_value));
    assert(openvn_set_int("score", 10));
    assert(openvn_get_int("score", &int_value) && int_value == 10);
    assert(openvn_set_int("score", -25));
    assert(openvn_get_int("score", &int_value) && int_value == -25);

    assert(openvn_set_bool("has_key", 1));
    assert(openvn_get_bool("has_key", &bool_value) && bool_value == 1);
    assert(openvn_set_bool("has_key", 0));
    assert(openvn_get_bool("has_key", &bool_value) && bool_value == 0);

    assert(openvn_set_string("player_name", "Alice"));
    assert(openvn_set_string("player_name", "Erik"));
    assert(openvn_get_string("player_name", string_value, sizeof(string_value)));
    assert(strcmp(string_value, "Erik") == 0);
    assert(!openvn_get_string("player_name", string_value, 4U));
    assert(!openvn_get_bool("score", &bool_value));

    memset(too_long, 'x', sizeof(too_long));
    too_long[sizeof(too_long) - 1U] = '\0';
    assert(!openvn_set_string("long", too_long));
    too_long[OPENVN_VARIABLE_STRING_MAX] = '\0';
    assert(openvn_set_string("maximum", too_long));
    assert(openvn_get_string("maximum", string_value, sizeof(string_value)));

    assert(openvn_set_string("greeting", "Hei, verden!"));
    assert(openvn_get_string("greeting", string_value, sizeof(string_value)));
    assert(strcmp(string_value, "Hei, verden!") == 0);

    openvn_variables_reset();
    for (index = 0U; index < OPENVN_MAX_VARIABLES; index++) {
        sprintf(name, "v%lu", (unsigned long)index);
        assert(openvn_set_int(name, (int32_t)index));
    }
    assert(!openvn_set_int("overflow", 1));
    assert(openvn_set_int("v0", 99));
    assert(openvn_get_int("v0", &int_value) && int_value == 99);

    return 0;
}
