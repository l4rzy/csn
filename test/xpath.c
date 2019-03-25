#include <csn.h>
#include "../src/internal.h"

void xpath_print(csn_xpath_t *xp) {
    csn_xpath_t *ptr = xp;
    while (ptr) {
        if (ptr->root) {
            printf("root ");
        } else {
            printf("-> %s[%d] ", ptr->tag.str, ptr->index);
        }
        ptr = ptr->next;
    }
    printf("\n");
}

#define X_PARSE(p) \
    do { \
        csn_xpath_t *r = csn_xpath_parse(p); \
        ASSERT(r); \
        puts(p); \
        xpath_print(r); \
    } while (0)

int main() {
    X_PARSE(CSN_HOT_VIDEOS_XPATH);
    X_PARSE(CSN_HOT_VN_XPATH);
    X_PARSE(CSN_HOT_USUK_XPATH);
    X_PARSE(CSN_SEARCH_XPATH);

    return 0;
}
