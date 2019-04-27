//
// Created by t5w0rd on 19-4-27.
//

#include <wchar.h>
#include <string.h>
#include <limits.h>
#include "tlang.h"


int CRB_mbstowcs_len(const char *src) {
    mbstate_t ps;
    memset(&ps, 0, sizeof(mbstate_t));
    int src_idx, dest_idx;
    for (src_idx = dest_idx = 0; src[src_idx] != '\0'; ) {
        int status = mbrtowc(NULL, &src[src_idx], MB_LEN_MAX, &ps);
        if (status < 0) {
            return status;
        }
        dest_idx++;
        src_idx += status;
    }
    return dest_idx;
}

void CRB_mbstowcs(const char *src, CRB_Char *dest) {
    int src_idx, dest_idx;
    int status;
    mbstate_t ps;

    memset(&ps, 0, sizeof(mbstate_t));
    for (src_idx = dest_idx = 0; src[src_idx] != '\0'; ) {
        status = mbrtowc(&dest[dest_idx], &src[src_idx], MB_LEN_MAX, &ps);
        dest_idx++;
        src_idx += status;
    }
    dest[dest_idx] = L'\0';
}

CRB_Char* CRB_mbstowcs_alloc(CRB_Interpreter *inter, CRB_LocalEnvironment *env, int line_number, const char *src) {
    int len = CRB_mbstowcs_len(src);
    if (len < 0) {
        return NULL;
//        crb_runtime_error(inter, env, line_number, BAD_MULTIBYTE_CHARACTER_ERR, CRB_MESSAGE_ARGUMENT_END);
    }
    CRB_Char* ret = MEM_malloc(sizeof(CRB_Char)*(len+1));
    CRB_mbstowcs(src, ret);
    return ret;
}

int CRB_wcstombs_len(const CRB_Char *src) {
    char dummy[MB_LEN_MAX];
    mbstate_t ps;
    memset(&ps, 0, sizeof(mbstate_t));
    int dest_idx;
    for (int src_idx = dest_idx = 0; src[src_idx] != L'\0'; ++src_idx) {
        int status = wcrtomb(dummy, src[src_idx], &ps);
        dest_idx += status;
    }
    return dest_idx;
}

void CRB_wcstombs(const CRB_Char *src, char *dest) {
    mbstate_t ps;
    memset(&ps, 0, sizeof(mbstate_t));
    int dest_idx;
    for (int src_idx = dest_idx = 0; src[src_idx] != '\0'; ++src_idx) {
        int status = wcrtomb(&dest[dest_idx], src[src_idx], &ps);
        dest_idx += status;
    }
    dest[dest_idx] = '\0';
}

char* CRB_wcstombs_alloc(const CRB_Char *src) {
    int len = CRB_wcstombs_len(src);
    char* ret = MEM_malloc(len + 1);
    CRB_wcstombs(src, ret);
    return ret;
}

char CRB_wctochar(CRB_Char src) {
    mbstate_t ps;
    char dest[MB_LEN_MAX];
    memset(&ps, 0, sizeof(mbstate_t));
    int status = wcrtomb(dest, src, &ps);
    DBG_assert(status == 1, ("wcrtomb status..%d\n", status));
    return dest[0];
}

int CRB_print_wcs(FILE *fp, const CRB_Char *str) {
    int mb_len = CRB_wcstombs_len(str);
    char* tmp = MEM_malloc(mb_len + 1);
    CRB_wcstombs(str, tmp);
    int result = fprintf(fp, "%s", tmp);
    MEM_free(tmp);
    return result;
}

int CRB_print_wcs_ln(FILE *fp, const CRB_Char *str) {
    int result = CRB_print_wcs(fp, str);
    fprintf(fp, "\n");
    return result;
}
