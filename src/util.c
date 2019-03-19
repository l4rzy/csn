#include "internal.h"

void *_xalloc(size_t size) {
    void *ret = malloc(size);
    if (!ret) {
        fatals("Could not allocate memory\n");
    }
    return ret;
}

void *_xcalloc(size_t size) {
    void *ret = calloc(size, 1);

    if (!ret) {
        fatals("Could not allocate memory\n");
    }
    return ret;
}

void *_xrealloc(void *ptr, size_t size) {
    ptr = realloc(ptr, size);
    if (!ptr) {
        fatals("Could not allocate memory\n");
    }
    return ptr;
}

buf_t *csn_buf_new(size_t size) {
    buf_t *ret = xalloc(sizeof(buf_t));

    ret->str = xcalloc(size+1);
    ret->len = 0;

    return ret;
}

char *csn_buf_write(buf_t *buf, const char *str) {
    int new_len = strlen(str);
    if (!buf) {
        logs("write to unallocated buf\n");
        buf = csn_buf_new(new_len +1);
    }

    // realloc if new str differs
    if (new_len != buf->len) {
        buf->str = xrealloc(buf->str, new_len+1);
    }
    buf->len = new_len;
    memcpy(buf->str, str, buf->len);
    buf->str[buf->len] = '\0';
    return buf->str;
}

char *csn_buf_append(buf_t *buf, const char *str) {
    int old = buf->len;
    int new_len = strlen(str);
    buf->len += new_len; // new len
    buf->str = xrealloc(buf->str, buf->len+1);

    memcpy(buf->str+old, str, new_len);
    buf->str[buf->len] = '\0';

    return buf->str;
}

int csn_buf_free(buf_t *buf) {
    free(buf->str);
    return 0;
}

/* ================= */
csn_result_t *csn_result_new(bool is_song) {
    csn_result_t *ret = xalloc(sizeof(csn_result_t));

    ret->next = NULL;
    ret->is_song = is_song;
    return ret;
}

int csn_result_free(csn_result_t *head) {
    csn_result_t *r = head;
    while (r->next != NULL) {
        free(r);
    }

    return 0;
}

