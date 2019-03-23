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

int main() {
    csn_xpath_t *r = csn_xpath_parse("/html/div/div[3]/img[2]");
    ASSERT(r);
    xpath_print(r);
}
