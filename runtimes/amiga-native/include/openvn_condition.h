#ifndef OPENVN_CONDITION_H
#define OPENVN_CONDITION_H

#include "openvn_variables.h"

typedef enum OpenVNConditionOperator {
    OPENVN_CONDITION_BOOL_TRUE = 0,
    OPENVN_CONDITION_BOOL_FALSE,
    OPENVN_CONDITION_EQUAL,
    OPENVN_CONDITION_NOT_EQUAL,
    OPENVN_CONDITION_LESS,
    OPENVN_CONDITION_LESS_EQUAL,
    OPENVN_CONDITION_GREATER,
    OPENVN_CONDITION_GREATER_EQUAL
} OpenVNConditionOperator;

typedef struct OpenVNCondition {
    const char *variable_name;
    OpenVNVariableType value_type;
    OpenVNConditionOperator operator;
    int bool_value;
    int32_t int_value;
    const char *string_value;
} OpenVNCondition;

int openvn_condition_evaluate(
    const OpenVNCondition *condition,
    int *result
);

#endif
