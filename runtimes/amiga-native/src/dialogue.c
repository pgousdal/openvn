#include "openvn_dialogue.h"

#include <string.h>

void openvn_dialogue_layout(
    unsigned int screen_width,
    unsigned int screen_height,
    OpenVNDialogueLayout *layout
) {
    int margin;
    int box_height;

    if (layout == 0) {
        return;
    }

    margin = 8;
    box_height = 72;
    if (screen_height < 96U) {
        box_height = (int)screen_height / 3;
    }

    layout->box_x = margin;
    layout->box_y = (int)screen_height - box_height - margin;
    layout->box_width = (int)screen_width - (margin * 2);
    layout->box_height = box_height;
    layout->text_x = layout->box_x + 8;
    layout->text_y = layout->box_y + 8;
    layout->text_width = layout->box_width - 16;
    layout->text_height = layout->box_height - 16;
}

static int append_char(
    char *output,
    size_t output_size,
    size_t *length,
    char value
) {
    if (*length + 1U >= output_size) {
        return 0;
    }
    output[*length] = value;
    *length += 1U;
    output[*length] = '\0';
    return 1;
}

int openvn_dialogue_wrap(
    const char *text,
    unsigned int max_columns,
    char *output,
    size_t output_size
) {
    const char *cursor;
    const char *word;
    size_t length;
    unsigned int column;
    unsigned int word_length;

    if (text == 0 || output == 0 || output_size == 0U ||
        max_columns == 0U) {
        return 0;
    }

    output[0] = '\0';
    cursor = text;
    length = 0U;
    column = 0U;

    while (*cursor != '\0') {
        if (*cursor == '\n') {
            if (!append_char(output, output_size, &length, '\n')) {
                return 0;
            }
            ++cursor;
            column = 0U;
            continue;
        }

        while (*cursor == ' ' || *cursor == '\t') {
            ++cursor;
        }
        if (*cursor == '\0') {
            break;
        }

        word = cursor;
        word_length = 0U;
        while (cursor[word_length] != '\0' &&
               cursor[word_length] != ' ' &&
               cursor[word_length] != '\t' &&
               cursor[word_length] != '\n') {
            ++word_length;
        }

        if (column != 0U && column + 1U + word_length > max_columns) {
            if (!append_char(output, output_size, &length, '\n')) {
                return 0;
            }
            column = 0U;
        } else if (column != 0U) {
            if (!append_char(output, output_size, &length, ' ')) {
                return 0;
            }
            ++column;
        }

        while (word_length > 0U) {
            if (column >= max_columns) {
                if (!append_char(output, output_size, &length, '\n')) {
                    return 0;
                }
                column = 0U;
            }
            if (!append_char(output, output_size, &length, *word)) {
                return 0;
            }
            ++word;
            --word_length;
            ++column;
        }

        cursor = word;
    }

    return 1;
}
