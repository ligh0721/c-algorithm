//
// Created by t5w0rd on 19-4-20.
//

#ifndef TLANG_TLEXER_H
#define TLANG_TLEXER_H


// string literal
char* crb_create_identifier(char *str);
void crb_open_string_literal(void);
void crb_add_string_literal(int letter);
void crb_reset_string_literal_buffer(void);
CRB_Char* crb_close_string_literal(void);

#define increment_line_number() (crb_get_current_interpreter()->current_line_number++)

void crb_set_source_string(const char** source);

void crb_set_readline(ReadLineModeParams* params);
int my_yyinput(char *buf, int max_size);


#endif //TLANG_TLEXER_H
