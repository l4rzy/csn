#include <csn.h>
#include "../src/internal.h"

void xpath_print(csn_xpath_t *xp) {
    csn_xpath_t *xptr = xp;
    while (xptr) {
        if (xptr->is_root) {
            printf("root ");
        } else {
            printf("-> %s[%d] ", xptr->tag->str, xptr->index);
        }
        xptr = xptr->next;
    }
    printf("\n");
}

#define X_PARSE(p) \
    do { \
        csn_xpath_t *r = csn_xpath_parse(p); \
        ASSERT(r); \
        puts(p); \
        xpath_print(r); \
        csn_xpath_free(r); \
    } while (0)

int main() {
    X_PARSE(CSN_HOT_VIDEOS_XPATH);
    X_PARSE(CSN_HOT_VN_XPATH);
    X_PARSE(CSN_HOT_USUK_XPATH);
    X_PARSE(CSN_SEARCH_XPATH);

    return 0;
}
