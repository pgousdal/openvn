#include "openvn_story.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static int read_file(const char *path, char **buffer_out) {
    FILE *file;
    long size;
    char *buffer;

    file = fopen(path, "rb");
    if (file == NULL) {
        return 0;
    }

    if (fseek(file, 0, SEEK_END) != 0) {
        fclose(file);
        return 0;
    }

    size = ftell(file);
    if (size < 0 || fseek(file, 0, SEEK_SET) != 0) {
        fclose(file);
        return 0;
    }

    buffer = (char *)malloc((size_t)size + 1U);
    if (buffer == NULL) {
        fclose(file);
        return 0;
    }

    if (fread(buffer, 1U, (size_t)size, file) != (size_t)size) {
        free(buffer);
        fclose(file);
        return 0;
    }

    buffer[size] = '\0';
    fclose(file);
    *buffer_out = buffer;
    return 1;
}

static const char *skip_space(const char *cursor) {
    while (*cursor == ' ' || *cursor == '\t' || *cursor == '\r' || *cursor == '\n') {
        cursor++;
    }
    return cursor;
}

static int extract_string(
    const char *object,
    const char *key,
    char *destination,
    size_t destination_size
) {
    char pattern[96];
    const char *cursor;
    const char *end;
    size_t length;

    snprintf(pattern, sizeof(pattern), "\"%s\"", key);
    cursor = strstr(object, pattern);
    if (cursor == NULL) {
        return 0;
    }

    cursor = strchr(cursor + strlen(pattern), ':');
    if (cursor == NULL) {
        return 0;
    }

    cursor = skip_space(cursor + 1);
    if (*cursor == 'n' && strncmp(cursor, "null", 4U) == 0) {
        destination[0] = '\0';
        return 1;
    }

    if (*cursor != '"') {
        return 0;
    }

    cursor++;
    end = cursor;
    while (*end != '\0' && *end != '"') {
        if (*end == '\\' && end[1] != '\0') {
            end += 2;
        } else {
            end++;
        }
    }

    if (*end != '"') {
        return 0;
    }

    length = (size_t)(end - cursor);
    if (length >= destination_size) {
        length = destination_size - 1U;
    }

    memcpy(destination, cursor, length);
    destination[length] = '\0';
    return 1;
}

static OpenVNNodeType node_type_from_string(const char *name) {
    if (strcmp(name, "text") == 0) return OPENVN_NODE_TEXT;
    if (strcmp(name, "choice") == 0) return OPENVN_NODE_CHOICE;
    if (strcmp(name, "jump") == 0) return OPENVN_NODE_JUMP;
    if (strcmp(name, "end") == 0) return OPENVN_NODE_END;
    if (strcmp(name, "scene") == 0) return OPENVN_NODE_SCENE;
    if (strcmp(name, "show") == 0) return OPENVN_NODE_SHOW;
    if (strcmp(name, "hide") == 0) return OPENVN_NODE_HIDE;
    if (strcmp(name, "music") == 0) return OPENVN_NODE_MUSIC;
    if (strcmp(name, "sound") == 0) return OPENVN_NODE_SOUND;
    return OPENVN_NODE_INVALID;
}

static const char *find_matching_brace(const char *start) {
    int depth = 0;
    int in_string = 0;
    const char *cursor = start;

    while (*cursor != '\0') {
        if (*cursor == '"' && (cursor == start || cursor[-1] != '\\')) {
            in_string = !in_string;
        } else if (!in_string) {
            if (*cursor == '{') depth++;
            if (*cursor == '}') {
                depth--;
                if (depth == 0) {
                    return cursor;
                }
            }
        }
        cursor++;
    }

    return NULL;
}

static int parse_choice_options(OpenVNStoryNode *node, const char *object) {
    const char *options;
    const char *cursor;
    const char *end;

    options = strstr(object, "\"options\"");
    if (options == NULL) {
        return 0;
    }

    cursor = strchr(options, '[');
    if (cursor == NULL) {
        return 0;
    }

    cursor++;
    while (*cursor != '\0' && *cursor != ']') {
        cursor = strchr(cursor, '{');
        if (cursor == NULL) {
            break;
        }

        end = find_matching_brace(cursor);
        if (end == NULL || node->option_count >= OPENVN_MAX_OPTIONS) {
            return 0;
        }

        {
            size_t length = (size_t)(end - cursor + 1);
            char *option_object = (char *)malloc(length + 1U);
            OpenVNChoiceOption *option;

            if (option_object == NULL) {
                return 0;
            }

            memcpy(option_object, cursor, length);
            option_object[length] = '\0';

            option = &node->options[node->option_count];
            if (!extract_string(
                    option_object,
                    "text",
                    option->text,
                    sizeof(option->text)
                ) ||
                !extract_string(
                    option_object,
                    "target",
                    option->target,
                    sizeof(option->target)
                )) {
                free(option_object);
                return 0;
            }

            node->option_count++;
            free(option_object);
        }

        cursor = end + 1;
    }

    return node->option_count > 0U;
}

static int parse_node(OpenVNStoryNode *node, const char *object) {
    char type_name[32];

    memset(node, 0, sizeof(*node));

    if (!extract_string(object, "id", node->id, sizeof(node->id)) ||
        !extract_string(object, "type", type_name, sizeof(type_name))) {
        return 0;
    }

    node->type = node_type_from_string(type_name);
    if (node->type == OPENVN_NODE_INVALID) {
        return 0;
    }

    switch (node->type) {
        case OPENVN_NODE_TEXT:
            return extract_string(object, "text", node->text, sizeof(node->text)) &&
                   extract_string(object, "next", node->next, sizeof(node->next));
        case OPENVN_NODE_CHOICE:
            return parse_choice_options(node, object);
        case OPENVN_NODE_JUMP:
            return extract_string(object, "target", node->target, sizeof(node->target));
        case OPENVN_NODE_END:
            return 1;
        case OPENVN_NODE_SCENE:
            return extract_string(
                       object,
                       "background",
                       node->argument1,
                       sizeof(node->argument1)
                   ) &&
                   extract_string(object, "next", node->next, sizeof(node->next));
        case OPENVN_NODE_SHOW:
            return extract_string(
                       object,
                       "character",
                       node->argument1,
                       sizeof(node->argument1)
                   ) &&
                   extract_string(
                       object,
                       "pose",
                       node->argument2,
                       sizeof(node->argument2)
                   ) &&
                   extract_string(object, "next", node->next, sizeof(node->next));
        case OPENVN_NODE_HIDE:
            return extract_string(
                       object,
                       "character",
                       node->argument1,
                       sizeof(node->argument1)
                   ) &&
                   extract_string(object, "next", node->next, sizeof(node->next));
        case OPENVN_NODE_MUSIC:
            return extract_string(
                       object,
                       "track",
                       node->argument1,
                       sizeof(node->argument1)
                   ) &&
                   extract_string(object, "next", node->next, sizeof(node->next));
        case OPENVN_NODE_SOUND:
            return extract_string(
                       object,
                       "sound",
                       node->argument1,
                       sizeof(node->argument1)
                   ) &&
                   extract_string(object, "next", node->next, sizeof(node->next));
        default:
            return 0;
    }
}

void openvn_story_reset(OpenVNStory *story) {
    if (story != NULL) {
        memset(story, 0, sizeof(*story));
    }
}

int openvn_story_load_file(OpenVNStory *story, const char *path) {
    char *document;
    const char *nodes;
    const char *cursor;

    if (story == NULL || path == NULL) {
        return 0;
    }

    openvn_story_reset(story);

    if (!read_file(path, &document)) {
        return 0;
    }

    if (!extract_string(document, "version", story->version, sizeof(story->version)) ||
        !extract_string(document, "entry", story->entry, sizeof(story->entry))) {
        free(document);
        return 0;
    }

    nodes = strstr(document, "\"nodes\"");
    if (nodes == NULL) {
        free(document);
        return 0;
    }

    cursor = strchr(nodes, '[');
    if (cursor == NULL) {
        free(document);
        return 0;
    }

    cursor++;
    while (*cursor != '\0' && *cursor != ']') {
        const char *end;
        size_t length;
        char *object;

        cursor = strchr(cursor, '{');
        if (cursor == NULL) {
            break;
        }

        end = find_matching_brace(cursor);
        if (end == NULL || story->node_count >= OPENVN_MAX_NODES) {
            free(document);
            return 0;
        }

        length = (size_t)(end - cursor + 1);
        object = (char *)malloc(length + 1U);
        if (object == NULL) {
            free(document);
            return 0;
        }

        memcpy(object, cursor, length);
        object[length] = '\0';

        if (!parse_node(&story->nodes[story->node_count], object)) {
            free(object);
            free(document);
            return 0;
        }

        story->node_count++;
        free(object);
        cursor = end + 1;
    }

    free(document);
    story->loaded = story->node_count > 0U;
    return story->loaded;
}

int openvn_story_find_node(const OpenVNStory *story, const char *id) {
    size_t index;

    if (story == NULL || id == NULL) {
        return -1;
    }

    for (index = 0U; index < story->node_count; index++) {
        if (strcmp(story->nodes[index].id, id) == 0) {
            return (int)index;
        }
    }

    return -1;
}

const OpenVNStoryNode *openvn_story_current(const OpenVNStory *story) {
    if (story == NULL || !story->loaded || story->current_index >= story->node_count) {
        return NULL;
    }

    return &story->nodes[story->current_index];
}

int openvn_story_start(OpenVNStory *story) {
    int index;

    if (story == NULL || !story->loaded) {
        return 0;
    }

    index = openvn_story_find_node(story, story->entry);
    if (index < 0) {
        return 0;
    }

    story->current_index = (size_t)index;
    story->ended = 0;
    return 1;
}

static int move_to(OpenVNStory *story, const char *id) {
    int index;

    if (id == NULL || id[0] == '\0') {
        story->ended = 1;
        return 1;
    }

    index = openvn_story_find_node(story, id);
    if (index < 0) {
        return 0;
    }

    story->current_index = (size_t)index;
    return 1;
}

int openvn_story_step(OpenVNStory *story) {
    const OpenVNStoryNode *node;

    node = openvn_story_current(story);
    if (node == NULL || story->ended) {
        return 0;
    }

    switch (node->type) {
        case OPENVN_NODE_TEXT:
        case OPENVN_NODE_SCENE:
        case OPENVN_NODE_SHOW:
        case OPENVN_NODE_HIDE:
        case OPENVN_NODE_MUSIC:
        case OPENVN_NODE_SOUND:
            return move_to(story, node->next);
        case OPENVN_NODE_JUMP:
            return move_to(story, node->target);
        case OPENVN_NODE_END:
            story->ended = 1;
            return 1;
        case OPENVN_NODE_CHOICE:
            return 0;
        default:
            return 0;
    }
}

int openvn_story_choose(OpenVNStory *story, size_t index) {
    const OpenVNStoryNode *node;

    node = openvn_story_current(story);
    if (node == NULL || node->type != OPENVN_NODE_CHOICE) {
        return 0;
    }

    if (index >= node->option_count) {
        return 0;
    }

    return move_to(story, node->options[index].target);
}

const char *openvn_story_status(const OpenVNStory *story) {
    const OpenVNStoryNode *node;

    if (story == NULL || !story->loaded) {
        return "UNLOADED";
    }

    if (story->ended) {
        return "ENDED";
    }

    node = openvn_story_current(story);
    if (node == NULL) {
        return "INVALID";
    }

    switch (node->type) {
        case OPENVN_NODE_TEXT: return "TEXT";
        case OPENVN_NODE_CHOICE: return "CHOICE";
        case OPENVN_NODE_JUMP: return "JUMP";
        case OPENVN_NODE_END: return "END";
        case OPENVN_NODE_SCENE: return "SCENE";
        case OPENVN_NODE_SHOW: return "SHOW";
        case OPENVN_NODE_HIDE: return "HIDE";
        case OPENVN_NODE_MUSIC: return "MUSIC";
        case OPENVN_NODE_SOUND: return "SOUND";
        default: return "INVALID";
    }
}
