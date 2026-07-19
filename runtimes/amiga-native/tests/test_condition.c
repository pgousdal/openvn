#include "openvn_condition.h"
#include "openvn_story.h"

#include <assert.h>
#include <limits.h>
#include <string.h>

static OpenVNCondition condition(
    const char *name,
    OpenVNVariableType type,
    OpenVNConditionOperator operator
) {
    OpenVNCondition value;

    memset(&value, 0, sizeof(value));
    value.variable_name = name;
    value.value_type = type;
    value.operator = operator;
    return value;
}

static void test_bool_conditions(void) {
    OpenVNCondition value;
    int result;

    openvn_variables_reset();
    assert(openvn_set_bool("flag", 1));

    value = condition(
        "flag",
        OPENVN_VARIABLE_BOOL,
        OPENVN_CONDITION_BOOL_TRUE
    );
    assert(openvn_condition_evaluate(&value, &result) && result);
    value.operator = OPENVN_CONDITION_BOOL_FALSE;
    assert(openvn_condition_evaluate(&value, &result) && !result);
    value.operator = OPENVN_CONDITION_EQUAL;
    value.bool_value = 1;
    assert(openvn_condition_evaluate(&value, &result) && result);
    value.operator = OPENVN_CONDITION_NOT_EQUAL;
    assert(openvn_condition_evaluate(&value, &result) && !result);
}

static void test_int_conditions(void) {
    static const OpenVNConditionOperator operators[] = {
        OPENVN_CONDITION_EQUAL,
        OPENVN_CONDITION_NOT_EQUAL,
        OPENVN_CONDITION_LESS,
        OPENVN_CONDITION_LESS_EQUAL,
        OPENVN_CONDITION_GREATER,
        OPENVN_CONDITION_GREATER_EQUAL
    };
    static const int expected[] = {1, 0, 0, 1, 0, 1};
    OpenVNCondition value;
    int result;
    size_t index;

    openvn_variables_reset();
    assert(openvn_set_int("score", 10));
    value = condition("score", OPENVN_VARIABLE_INT, OPENVN_CONDITION_EQUAL);
    value.int_value = 10;
    for (index = 0U; index < sizeof(operators) / sizeof(operators[0]); index++) {
        value.operator = operators[index];
        assert(openvn_condition_evaluate(&value, &result));
        assert(result == expected[index]);
    }

    assert(openvn_set_int("score", INT32_MIN));
    value.operator = OPENVN_CONDITION_EQUAL;
    value.int_value = INT32_MIN;
    assert(openvn_condition_evaluate(&value, &result) && result);
    assert(openvn_set_int("score", INT32_MAX));
    value.int_value = INT32_MAX;
    assert(openvn_condition_evaluate(&value, &result) && result);
}

static void test_string_conditions(void) {
    OpenVNCondition value;
    int result;

    openvn_variables_reset();
    assert(openvn_set_string("player_name", "Alice"));
    value = condition(
        "player_name",
        OPENVN_VARIABLE_STRING,
        OPENVN_CONDITION_EQUAL
    );
    value.string_value = "Alice";
    assert(openvn_condition_evaluate(&value, &result) && result);
    value.operator = OPENVN_CONDITION_NOT_EQUAL;
    assert(openvn_condition_evaluate(&value, &result) && !result);
}

static void test_errors_do_not_mutate_store(void) {
    OpenVNCondition value;
    int32_t stored;
    int result;

    openvn_variables_reset();
    assert(openvn_set_int("score", 10));
    result = 7;
    value = condition("missing", OPENVN_VARIABLE_INT, OPENVN_CONDITION_EQUAL);
    assert(!openvn_condition_evaluate(&value, &result));
    assert(result == 7);
    value = condition("score", OPENVN_VARIABLE_BOOL, OPENVN_CONDITION_EQUAL);
    assert(!openvn_condition_evaluate(&value, &result));
    value = condition(
        "score",
        OPENVN_VARIABLE_INT,
        (OpenVNConditionOperator)99
    );
    assert(!openvn_condition_evaluate(&value, &result));
    assert(openvn_get_int("score", &stored) && stored == 10);
}

static void test_branch_selection(void) {
    OpenVNGeneratedNode nodes[5];
    OpenVNGeneratedStory story;
    OpenVNStoryState state;

    memset(nodes, 0, sizeof(nodes));
    nodes[0].id = "condition";
    nodes[0].type = OPENVN_NODE_CONDITION;
    nodes[0].true_target = "true";
    nodes[0].false_target = "false";
    nodes[1].id = "true";
    nodes[1].type = OPENVN_NODE_CONDITION;
    nodes[1].true_target = "nested-true";
    nodes[1].false_target = "nested-false";
    nodes[2].id = "false";
    nodes[2].type = OPENVN_NODE_END;
    nodes[3].id = "nested-true";
    nodes[3].type = OPENVN_NODE_END;
    nodes[4].id = "nested-false";
    nodes[4].type = OPENVN_NODE_END;
    story.version = "0.4";
    story.entry = "condition";
    story.nodes = nodes;
    story.node_count = 5U;

    assert(openvn_story_attach(&state, &story));
    assert(openvn_story_start(&state));
    assert(openvn_story_branch(&state, 1));
    assert(strcmp(openvn_story_current(&state)->id, "true") == 0);
    assert(openvn_story_branch(&state, 0));
    assert(strcmp(openvn_story_current(&state)->id, "nested-false") == 0);
    assert(openvn_story_start(&state));
    assert(openvn_story_branch(&state, 0));
    assert(strcmp(openvn_story_current(&state)->id, "false") == 0);
}

int main(void) {
    test_bool_conditions();
    test_int_conditions();
    test_string_conditions();
    test_errors_do_not_mutate_store();
    test_branch_selection();
    return 0;
}
