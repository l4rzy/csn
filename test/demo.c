#include "csn.h"

int print_entry(csn_result_t *r) {
    return 1;
}

int main(int argc, char *argv[]) {
    if (argc != 2) {
        printf("%s [search string]\n", argv[0]);
        return 0;
    }
    csn_ctx_t *ctx = csn_init(NULL);
    csn_result_t *result = csn_search(ctx, argv[1],
            SEARCH_ARTIST | SEARCH_SONG, 10);

   if (!result) {
        goto _exit;
    }

    csn_result_t *head = result;
    while (head) {
        print_entry(head);
        puts("=====");
        head = head->next;
    }
    csn_result_free(result);

_exit:
    csn_free(ctx);
    return 0;
}
