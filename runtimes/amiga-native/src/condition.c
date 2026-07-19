#include "openvn_condition.h"

#include <stdio.h>
#include <string.h>

static const char *operator_name(OpenVNConditionOperator operator) {
    switch (operator) {
        case OPENVN_CONDITION_BOOL_TRUE: return "== true";
        case OPENVN_CONDITION_BOOL_FALSE: return "== false";
        case OPENVN_CONDITION_EQUAL: return "==";
        case OPENVN_CONDITION_NOT_EQUAL: return "!=";
        case OPENVN_CONDITION_LESS: return "<";
        case OPENVN_CONDITION_LESS_EQUAL: return "<=";
        case OPENVN_CONDITION_GREATER: return ">";
        case OPENVN_CONDITION_GREATER_EQUAL: return ">=";
        default: return "?";
    }
}

static void log_condition(const OpenVNCondition *condition) {
    if (condition->operator == OPENVN_CONDITION_BOOL_TRUE ||
        condition->operator == OPENVN_CONDITION_BOOL_FALSE) {
        printf(
            "CONDITION %s %s\n",
            condition->variable_name,
            operator_name(condition->operator)
        );
    } else if (condition->value_type == OPENVN_VARIABLE_BOOL) {
        printf(
            "CONDITION %s %s %s\n",
            condition->variable_name,
            operator_name(condition->operator),
            condition->bool_value ? "true" : "false"
        );
    } else if (condition->value_type == OPENVN_VARIABLE_INT) {
        printf(
            "CONDITION %s %s %ld\n",
            condition->variable_name,
            operator_name(condition->operator),
            (long)condition->int_value
        );
    } else {
        printf(
            "CONDITION %s %s \"%s\"\n",
            condition->variable_name,
            operator_name(condition->operator),
            condition->string_value
        );
    }
}

static int evaluate_bool(const OpenVNCondition *condition, int *result) {
    int value;

    if (!openvn_get_bool(condition->variable_name, &value)) return 0;
    switch (condition->operator) {
        case OPENVN_CONDITION_BOOL_TRUE:
            *result = value != 0;
            return 1;
        case OPENVN_CONDITION_BOOL_FALSE:
            *result = value == 0;
            return 1;
        case OPENVN_CONDITION_EQUAL:
            *result = value == (condition->bool_value != 0);
            return 1;
        case OPENVN_CONDITION_NOT_EQUAL:
            *result = value != (condition->bool_value != 0);
            return 1;
        default: return 0;
    }
}

static int evaluate_int(const OpenVNCondition *condition, int *result) {
    int32_t value;

    if (!openvn_get_int(condition->variable_name, &value)) return 0;
    switch (condition->operator) {
        case OPENVN_CONDITION_EQUAL:
            *result = value == condition->int_value;
            return 1;
        case OPENVN_CONDITION_NOT_EQUAL:
            *result = value != condition->int_value;
            return 1;
        case OPENVN_CONDITION_LESS:
            *result = value < condition->int_value;
            return 1;
        case OPENVN_CONDITION_LESS_EQUAL:
            *result = value <= condition->int_value;
            return 1;
        case OPENVN_CONDITION_GREATER:
            *result = value > condition->int_value;
            return 1;
        case OPENVN_CONDITION_GREATER_EQUAL:
            *result = value >= condition->int_value;
            return 1;
        default: return 0;
    }
}

static int evaluate_string(const OpenVNCondition *condition, int *result) {
    char value[OPENVN_VARIABLE_STRING_MAX + 1];
    int comparison;

    if (condition->string_value == 0 ||
        !openvn_get_string(condition->variable_name, value, sizeof(value))) {
        return 0;
    }
    comparison = strcmp(value, condition->string_value);
    if (condition->operator == OPENVN_CONDITION_EQUAL) {
        *result = comparison == 0;
        return 1;
    }
    if (condition->operator == OPENVN_CONDITION_NOT_EQUAL) {
        *result = comparison != 0;
        return 1;
    }
    return 0;
}

int openvn_condition_evaluate(
    const OpenVNCondition *condition,
    int *result
) {
    OpenVNVariableType actual_type;
    int evaluated;

    if (condition == 0 || result == 0 || condition->variable_name == 0 ||
        condition->variable_name[0] == '\0') {
        printf("CONDITION error: invalid input\n");
        return 0;
    }
    if (!openvn_variable_type(condition->variable_name, &actual_type)) {
        printf(
            "CONDITION error: unknown variable %s\n",
            condition->variable_name
        );
        return 0;
    }
    if (actual_type != condition->value_type) {
        printf(
            "CONDITION error: type mismatch for %s\n",
            condition->variable_name
        );
        return 0;
    }

    log_condition(condition);
    if (actual_type == OPENVN_VARIABLE_BOOL) {
        evaluated = evaluate_bool(condition, result);
    } else if (actual_type == OPENVN_VARIABLE_INT) {
        evaluated = evaluate_int(condition, result);
    } else if (actual_type == OPENVN_VARIABLE_STRING) {
        evaluated = evaluate_string(condition, result);
    } else {
        evaluated = 0;
    }
    if (!evaluated) {
        printf(
            "CONDITION error: invalid operator for %s\n",
            condition->variable_name
        );
        return 0;
    }
    printf("CONDITION result = %s\n", *result ? "true" : "false");
    return 1;
}
