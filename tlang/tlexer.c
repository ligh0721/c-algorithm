//
// Created by t5w0rd on 19-4-20.
//

#include <string.h>
#include "tlexer.h"
#include "tmisc.h"
#include "terror.h"

// string
#define STRING_ALLOC_SIZE       (256)

static char *st_string_literal_buffer = NULL;
static int st_string_literal_buffer_size = 0;
static int st_string_literal_buffer_alloc_size = 0;

void crb_open_string_literal(void) {
    st_string_literal_buffer_size = 0;
}

void crb_add_string_literal(int letter) {
    if (st_string_literal_buffer_size == st_string_literal_buffer_alloc_size) {
        st_string_literal_buffer_alloc_size += STRING_ALLOC_SIZE;
        st_string_literal_buffer = MEM_realloc(st_string_literal_buffer, st_string_literal_buffer_alloc_size);
    }
    st_string_literal_buffer[st_string_literal_buffer_size] = letter;
    ++st_string_literal_buffer_size;
}

void
crb_reset_string_literal_buffer(void)
{
    MEM_free(st_string_literal_buffer);
    st_string_literal_buffer = NULL;
    st_string_literal_buffer_size = 0;
    st_string_literal_buffer_alloc_size = 0;
}

CRB_Char* crb_close_string_literal(void) {
    CRB_Char *new_str;
    int new_str_len;

    crb_add_string_literal('\0');
    new_str_len = CRB_mbstowcs_len(st_string_literal_buffer);
    if (new_str_len < 0) {
        crb_compile_error(BAD_MULTIBYTE_CHARACTER_IN_COMPILE_ERR,
                          CRB_MESSAGE_ARGUMENT_END);
    }
    new_str = crb_malloc(sizeof(CRB_Char) * (new_str_len+1));
    CRB_mbstowcs(st_string_literal_buffer, new_str);

    return new_str;
}

char* crb_create_identifier(char *str) {
    char* new_str = crb_malloc(strlen(str) + 1);
    strcpy(new_str, str);
    return new_str;
}

// lex
extern FILE* yyin;
extern char* yytext;

static int file_input(char *buf, int max_size) {
    if (feof(yyin)) {
        return 0;
    }

    for (int len = 0; len < max_size; len++) {
        int ch = getc(yyin);
        if (ch == EOF) {
            return len;
        }
        buf[len] = ch;
    }
    return 0;
}

static const char** st_source_string;
static int st_current_source_line;
static int st_current_char_index;

void crb_set_source_string(const char** source) {
    st_source_string = source;
    st_current_source_line = 0;
    st_current_char_index = 0;
}

static int string_input(char *buf, int max_size) {
    if (st_source_string[st_current_source_line] == NULL) {
        return 0;
    }

    if (st_source_string[st_current_source_line][st_current_char_index] == '\0') {
        ++st_current_source_line;
        st_current_char_index = 0;
    }

    if (st_source_string[st_current_source_line] == NULL) {
        return 0;
    }

    int len = smaller(strlen(st_source_string[st_current_source_line]) - st_current_char_index, max_size);
    strncpy(buf, &st_source_string[st_current_source_line][st_current_char_index], len);
    st_current_char_index += len;
    return len;
}

int my_yyinput(char *buf, int max_size) {
    int result;
    switch (crb_get_current_interpreter()->input_mode) {
        case CRB_FILE_INPUT_MODE:
            result = file_input(buf, max_size);
            break;
        case CRB_STRING_INPUT_MODE:
            result = string_input(buf, max_size);
            break;
        default:
            DBG_panic(("bad default. input_mode..%d\n", crb_get_current_interpreter()->input_mode));
    }

    return result;
}

int yywrap(void) {
    return 1;
}

int yyerror(char const* str) {
    crb_compile_error(PARSE_ERR, CRB_STRING_MESSAGE_ARGUMENT, "token", yytext, CRB_MESSAGE_ARGUMENT_END);
    return 0;
}
