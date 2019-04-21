//
// Created by t5w0rd on 19-4-21.
//

#include <assert.h>
#include <string.h>
#include <stdio.h>
#include <stdarg.h>
#include "wstring.h"

#define TEMP_STRING_LENGTH 64


struct wstring {
    wchar_t* data;
    long length;
};

WSTRING* open_wstring() {
    struct wstring* ret = NEW(struct wstring);
    assert(ret != NULL);
    ret->data = L"";
    ret->length = 0;
    return ret;
}

WSTRING* open_wstring_with_data(const wchar_t* src) {
    struct wstring* ret = NEW(struct wstring);
    assert(ret != NULL);
    ret->length = wcslen(src);
    if (ret->length == 0) {
        ret->data = L"";
        return ret;
    }
    ret->data = NEW3(wchar_t, ret->length+1);
    wcsncpy(ret->data, src, ret->length+1);
    return ret;
}

WSTRING* open_wstring_with_format(const wchar_t* fmt, ...) {
    va_list ap;
    struct wstring* ret = NEW(struct wstring);
    assert(ret != NULL);
    va_start(ap, fmt);
    ret->data = NEW3(wchar_t, TEMP_STRING_LENGTH+1);
    ret->length = vswprintf(ret->data, TEMP_STRING_LENGTH+1, fmt, ap);
    if (ret->length == 0) {
        DELETE(ret->data);
        ret->data = L"";
    } else if (ret->length+1 < TEMP_STRING_LENGTH) {
        ret->data = RENEW3(ret->data, wchar_t, ret->length+1);
    } else if (ret->length+1 > TEMP_STRING_LENGTH) {
        DELETE(ret->data);
        ret->data = NEW3(wchar_t, ret->length+1);
        vswprintf(ret->data, ret->length+1, fmt, ap);
    }
    va_end(ap);
    return ret;
}

void close_wstring(WSTRING* str) {
    assert(str != NULL);
    if (str->length != 0) {
        DELETE(str->data);
    }
    DELETE(str);
}

long wstring_len(WSTRING* str) {
    assert(str != NULL);
    return str->length;
}

wchar_t* wstring_data(WSTRING* str) {
    assert(str != NULL);
    return str->data;
}

void wstring_clear(WSTRING* str) {
    assert(str != NULL);
    if (str->length == 0) {
        return;
    }
    DELETE(str->data);
    str->length = 0;
}

int wstring_empty(WSTRING* str) {
    assert(str != NULL);
    return str->length == 0;
}

WSTRING* wstring_cpy(WSTRING* str, WSTRING* src) {
    assert(str != NULL && src != NULL);
    if (str == src) {
        return str;
    }
    if (src->length == 0) {
        if (str->length != 0) {
            DELETE(str->data);
            str->data = L"";
            str->length = 0;
        }
    } else {
        if (str->length != 0) {
            DELETE(str->data);
        }
        str->data = NEW3(wchar_t, src->length+1);
        str->length = src->length;
        wcsncpy(str->data, src->data, src->length+1);
    }
    return str;
}

int wstring_cmp(WSTRING* str, WSTRING* str2) {
    assert(str != NULL && str2 != NULL);
    if (str == str2) {
        return 0;
    }
    return wcsncmp(str->data, str2->data, (str->length<str2->length?str->length:str2->length)+1);
}

struct string {
    char* data;
    long length;
};

STRING* open_string() {
    struct string* ret = NEW(struct string);
    assert(ret != NULL);
    ret->data = "";
    ret->length = 0;
    return ret;
}

STRING* open_string_with_data(const char* src) {
    struct string* ret = NEW(struct string);
    assert(ret != NULL);
    ret->length = strlen(src);
    if (ret->length == 0) {
        ret->data = "";
        return ret;
    }
    ret->data = NEW3(char, ret->length+1);
    strncpy(ret->data, src, ret->length+1);
    return ret;
}

STRING* open_string_with_format(const char* fmt, ...) {
    va_list ap;
    struct string* ret = NEW(struct string);
    assert(ret != NULL);
    va_start(ap, fmt);
    ret->data = NEW3(char, TEMP_STRING_LENGTH+1);
    ret->length = vsnprintf(ret->data, TEMP_STRING_LENGTH+1, fmt, ap);
    if (ret->length == 0) {
        DELETE(ret->data);
        ret->data = "";
    } else if (ret->length+1 < TEMP_STRING_LENGTH) {
        ret->data = RENEW3(ret->data, char, ret->length+1);
    } else if (ret->length+1 > TEMP_STRING_LENGTH) {
        DELETE(ret->data);
        ret->data = NEW3(char, ret->length+1);
        vsnprintf(ret->data, ret->length+1, fmt, ap);
    }
    va_end(ap);
    return ret;
}

void close_string(STRING* str) {
    assert(str != NULL);
    if (str->length != 0) {
        DELETE(str->data);
    }
    DELETE(str);
}

long string_len(STRING* str) {
    assert(str != NULL);
    return str->length;
}

char* string_data(STRING* str) {
    assert(str != NULL);
    return str->data;
}

void string_clear(STRING* str) {
    assert(str != NULL);
    if (str->length == 0) {
        return;
    }
    DELETE(str->data);
    str->length = 0;
}

int string_empty(STRING* str) {
    assert(str != NULL);
    return str->length == 0;
}

STRING* string_cpy(STRING* str, STRING* src) {
    assert(str != NULL && src != NULL);
    if (str == src) {
        return str;
    }
    if (src->length == 0) {
        if (str->length != 0) {
            DELETE(str->data);
            str->data = "";
            str->length = 0;
        }
    } else {
        if (str->length != 0) {
            DELETE(str->data);
        }
        str->data = NEW3(char, src->length+1);
        str->length = src->length;
        strncpy(str->data, src->data, src->length+1);
    }
    return str;
}

int string_cmp(STRING* str, STRING* str2) {
    assert(str != NULL && str2 != NULL);
    if (str == str2) {
        return 0;
    }
    return strncmp(str->data, str2->data, (str->length<str2->length?str->length:str2->length)+1);
}
