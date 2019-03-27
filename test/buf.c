#include <csn.h>
#include <assert.h>
#include "../src/internal.h"

void print_buf(buf_t *b) {
    printf("\"%s\"\nlen: %d\n\n", b->str, b->len);
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
    buf_t *buf = csn_buf_from_str(saying);
    ASSERT(buf->len == strlen(saying));

    temp = construct_sanitized_string('A', 422);
    csn_buf_append(buf, temp);
    ASSERT(buf->len == strlen(saying) + 422);
    print_buf(buf);
    free(temp);

    temp = construct_sanitized_string('B', 1024);
    csn_buf_write(buf, temp);
    ASSERT(buf->len == 1024);
    print_buf(buf);
    free(temp);

    csn_buf_append_char(buf, 'X');
    ASSERT(buf->len == 1025);
    ASSERT(buf->str[buf->len -1] == 'X');
    print_buf(buf);

    csn_buf_write_char(buf, 'Z');
    ASSERT(buf->len == 1);
    ASSERT(buf->str[1] == '\0');
    print_buf(buf);

    csn_buf_free(buf);
}

