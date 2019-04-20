#include "internal.h"
#include <ctype.h>

/* TO CREATE NEW
 */
// new buffer that has size of `size`
buf_t *buf_new_size(size_t size) {
    buf_t *ret = xalloc(sizeof(buf_t));

    ret->str = xcalloc(size + 1);
    ret->len = 0;

    return ret;
}

// new buffer from memory, then set the last char to '\0'
buf_t *buf_new_mem(const char *mem, size_t size) {
    buf_t *ret = buf_new_size(size);
    ret->len = size;
    memcpy(ret->str, mem, size);

    return ret;
}

// new buffer from C string
buf_t *buf_new_str(const char *str) {
    int len = strlen(str);
    buf_t *ret = buf_new_mem(str, len);

    return ret;
}

// new buffer that possesses an existed string
buf_t *buf_new_possess(char *str) {
    int len = strlen(str);
    buf_t *buf = xalloc(sizeof(buf_t));
    buf->len = len;
    buf->str = str;

    return buf;
}

/* TO WRITE
 */
char *buf_write_char(buf_t *buf, const char c) {
    buf->str = xrealloc(buf->str, 2);
    buf->str[0] = c;
    buf->len = 1;
    buf->str[1] = '\0';

    return buf->str;
}

char *buf_write_mem(buf_t *buf, const char *mem, size_t size) {
    buf->str = xrealloc(buf->str, size + 1);
    memcpy(buf->str, mem, size);
    buf->str[size] = '\0';
    buf->len = size;

    return buf->str;
}

char *buf_write_str(buf_t *buf, const char *str) {
    size_t len = strlen(str);
    buf_write_mem(buf, str, len);

    return buf->str;
}

/* TO APPEND TO END
 */

char *buf_append_char(buf_t *buf, const char c) {
    buf->str = xrealloc(buf->str, buf->len + 2);
    buf->str[buf->len] = c;
    buf->str[++(buf->len)] = '\0';

    return buf->str;
}

char *buf_append_mem(buf_t *buf, const char *mem, size_t size) {
    size_t old_pos = buf->len;
    buf->len += size;
    buf->str = xrealloc(buf->str, buf->len + 1);

    memmove(buf->str + old_pos, mem, size);
    buf->str[buf->len] = '\0';

    return buf->str;
}

char *buf_append_str(buf_t *buf, const char *str) {
    size_t len = strlen(str);
    buf_append_mem(buf, str, len);

    return buf->str;
}

int buf_trim(buf_t *buf) {
    char *head, *tail;

    // trim for head
    head = buf->str;
    while (1) {
        if (isspace(*head)) {
            ++head;
            continue;
        }
        else {
            break;
        }
    }
    logf("head at %p\n", head);

    // trim from tail
    tail = buf->str + buf->len - 1; // last char
    while (1) {
        if (isspace(*tail)) {
            --tail;
            continue;
        }
        else {
            break;
        }
    }
    logf("tail at %p\n", tail);

    // edge case
    if (tail < head) {
        buf->str = xrealloc(buf->str, 1);
        buf->str[0] = '\0';
        buf->len = 0;
        return 0;
    }

    buf->len = tail - head + 1;
    memmove(buf->str, head, buf->len);
    buf->str = xrealloc(buf->str, buf->len+1);
    // set last char to null
    buf->str[buf->len] = '\0';
    return 0;
}

int buf_free(buf_t *buf) {
    if (buf) {
        free(buf->str);
        free(buf);
        return 0;
    }
    return 0;
}
