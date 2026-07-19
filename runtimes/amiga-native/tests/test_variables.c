#include "openvn_variables.h"

#include "test_check.h"
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
    OPENVN_TEST_CHECK(!openvn_get_int("unknown", &int_value));
    OPENVN_TEST_CHECK(openvn_set_int("score", 10));
    OPENVN_TEST_CHECK(openvn_get_int("score", &int_value) && int_value == 10);
    OPENVN_TEST_CHECK(openvn_set_int("score", -25));
    OPENVN_TEST_CHECK(openvn_get_int("score", &int_value) && int_value == -25);

    OPENVN_TEST_CHECK(openvn_set_bool("has_key", 1));
    OPENVN_TEST_CHECK(openvn_get_bool("has_key", &bool_value) && bool_value == 1);
    OPENVN_TEST_CHECK(openvn_set_bool("has_key", 0));
    OPENVN_TEST_CHECK(openvn_get_bool("has_key", &bool_value) && bool_value == 0);

    OPENVN_TEST_CHECK(openvn_set_string("player_name", "Alice"));
    OPENVN_TEST_CHECK(openvn_set_string("player_name", "Erik"));
    OPENVN_TEST_CHECK(openvn_get_string("player_name", string_value, sizeof(string_value)));
    OPENVN_TEST_CHECK(strcmp(string_value, "Erik") == 0);
    OPENVN_TEST_CHECK(!openvn_get_string("player_name", string_value, 4U));
    OPENVN_TEST_CHECK(!openvn_get_bool("score", &bool_value));

    memset(too_long, 'x', sizeof(too_long));
    too_long[sizeof(too_long) - 1U] = '\0';
    OPENVN_TEST_CHECK(!openvn_set_string("long", too_long));
    too_long[OPENVN_VARIABLE_STRING_MAX] = '\0';
    OPENVN_TEST_CHECK(openvn_set_string("maximum", too_long));
    OPENVN_TEST_CHECK(openvn_get_string("maximum", string_value, sizeof(string_value)));

    OPENVN_TEST_CHECK(openvn_set_string("greeting", "Hei, verden!"));
    OPENVN_TEST_CHECK(openvn_get_string("greeting", string_value, sizeof(string_value)));
    OPENVN_TEST_CHECK(strcmp(string_value, "Hei, verden!") == 0);

    openvn_variables_reset();
    for (index = 0U; index < OPENVN_MAX_VARIABLES; index++) {
        sprintf(name, "v%lu", (unsigned long)index);
        OPENVN_TEST_CHECK(openvn_set_int(name, (int32_t)index));
    }
    OPENVN_TEST_CHECK(!openvn_set_int("overflow", 1));
    OPENVN_TEST_CHECK(openvn_set_int("v0", 99));
    OPENVN_TEST_CHECK(openvn_get_int("v0", &int_value) && int_value == 99);

    return 0;
}
