#ifndef OPENVN_VARIABLES_H
#define OPENVN_VARIABLES_H

#include <stddef.h>
#include <stdint.h>

#ifndef OPENVN_MAX_VARIABLES
#define OPENVN_MAX_VARIABLES 32
#endif

#ifndef OPENVN_VARIABLE_NAME_MAX
#define OPENVN_VARIABLE_NAME_MAX 31
#endif

#ifndef OPENVN_VARIABLE_STRING_MAX
#define OPENVN_VARIABLE_STRING_MAX 127
#endif

void openvn_variables_reset(void);
int openvn_set_bool(const char *name, int value);
int openvn_get_bool(const char *name, int *value);
int openvn_set_int(const char *name, int32_t value);
int openvn_get_int(const char *name, int32_t *value);
int openvn_set_string(const char *name, const char *value);
int openvn_get_string(const char *name, char *value, size_t value_size);

#endif
