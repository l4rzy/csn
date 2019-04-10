#include "internal.h"
#include <ctype.h>

buf_t *csn_buf_new(size_t size) {
    buf_t *ret = xalloc(sizeof(buf_t));

    ret->str = xcalloc(size + 1);
    ret->len = 0;

    return ret;
}

buf_t *csn_buf_from_str(const char *str) {
    int len = strlen(str);
    buf_t *buf = csn_buf_new(len + 1);

    memcpy(buf->str, str, len);
    buf->len = len;
    return buf;
}

char *csn_buf_write(buf_t *buf, const char *str) {
    int new_len = strlen(str);
    if (!buf) {
        logs("write to unallocated buf\n");
        buf = csn_buf_new(new_len + 1);
    }

    // realloc if new str differs
    if (new_len != buf->len) {
        buf->str = xrealloc(buf->str, new_len + 1);
    }
    buf->len = new_len;
    memcpy(buf->str, str, buf->len);
    buf->str[buf->len] = '\0';
    return buf->str;
}

char *csn_buf_write_char(buf_t *buf, const char c) {
    // write a char at 0 position without realloc
    buf->str[0] = c;
    buf->len = 1;
    buf->str[1] = '\0';

    return buf->str;
}

char *csn_buf_append(buf_t *buf, const char *str) {
    int old = buf->len;
    int new_len = strlen(str);
    buf->len += new_len; // new len
    buf->str = xrealloc(buf->str, buf->len + 1);

    memcpy(buf->str + old, str, new_len);
    buf->str[buf->len] = '\0';

    return buf->str;
}

char *csn_buf_append_char(buf_t *buf, const char c) {
    buf->str = xrealloc(buf->str, buf->len + 2);
    buf->str[buf->len] = c;
    buf->str[++(buf->len)] = '\0';

    return buf->str;
}

int csn_buf_trim(buf_t *buf) {
    return 0;
}

int csn_buf_free(buf_t *buf) {
    if (buf) {
        free(buf->str);
        free(buf);
        return 0;
    }
    return 0;
}
