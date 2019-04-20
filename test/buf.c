#include <csn.h>
#include "../src/internal.h"

void print_buf(buf_t *b) {
    printf("\"%s\"\nlen: %ld\n\n", b->str, b->len);
}

char *construct_sanitized_string(char c, int len) {
    char *r = xalloc(len+1);
    memset(r, c, len);
    r[len] = '\0';

    return r;
}

int main() {
    char *temp;
    const char *saying = "The mark of an immature man is that he wants to \
die nobly for a cause, while the mark of a mature man is that he \
wants to live humbly for one.";
    int saying_len = strlen(saying);

    buf_t *buf = buf_new_str(saying);
    ASSERT(!memcmp(buf->str, saying, saying_len));
    ASSERT(buf->len == saying_len);

    buf_write_char(buf, 'A');
    ASSERT(buf->len == 1);

    temp = construct_sanitized_string('B', 1024);
    buf_write_str(buf, temp);
    ASSERT(buf->len == 1024);
    print_buf(buf);
    free(temp);

    temp = construct_sanitized_string('A', 422);
    buf_append_str(buf, temp);
    ASSERT(buf->len == 1024 + 422);
    print_buf(buf);
    free(temp);

    buf_append_char(buf, 'X');
    ASSERT(buf->len == 1024 + 422 + 1);
    ASSERT(buf->str[buf->len -1] == 'X');
    print_buf(buf);
    buf_free(buf);

    temp = construct_sanitized_string('~', 1024);
    buf = buf_new_possess(temp);
    print_buf(buf);
    ASSERT(buf->len == 1024);
    // no need to free temp since it was possessed by buf

    temp = construct_sanitized_string('\n', 1024);
    temp[1] = 'a';
    buf_write_str(buf, temp);
    ASSERT(buf->len == 1024);
    buf_trim(buf);
    print_buf(buf);
    ASSERT(buf->len == 1);
    free(temp);

    temp = construct_sanitized_string(' ', 1024);
    temp[1] = 'a';
    buf_write_str(buf, temp);
    ASSERT(buf->len == 1024);
    buf_trim(buf);
    print_buf(buf);
    ASSERT(buf->len == 1);

    buf_free(buf);
    free(buf);
}

