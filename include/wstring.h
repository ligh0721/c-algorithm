//
// Created by t5w0rd on 19-4-21.
//

#ifndef ALGORITHM_WSTRING_H
#define ALGORITHM_WSTRING_H

#include "algorithm.h"
#include <wchar.h>


typedef struct wstring WSTRING;

WSTRING* open_wstring();
WSTRING* open_wstring_with_data(const wchar_t* src);
WSTRING* open_wstring_with_format(const wchar_t* fmt, ...);
void close_wstring(WSTRING* str);
long wstring_len(WSTRING* str);
wchar_t* wstring_data(WSTRING* str);
void wstring_clear(WSTRING* str);
int wstring_empty(WSTRING* str);
WSTRING* wstring_cpy(WSTRING* str, WSTRING* src);
int wstring_cmp(WSTRING* str, WSTRING* str2);

typedef struct string STRING;

STRING* open_string();
STRING* open_string_with_data(const char* src);
STRING* open_string_with_format(const char* fmt, ...);
void close_string(STRING* str);
long string_len(STRING* str);
char* string_data(STRING* str);
void string_clear(STRING* str);
int string_empty(STRING* str);
STRING* string_cpy(STRING* str, STRING* src);
int string_cmp(STRING* str, STRING* str2);

#endif //ALGORITHM_WSTRING_H
