#include "csn.h"

int print_entry(csn_result_t *r) {
    if (r->is_song)
        printf("name: %s\nlink: %s\n", r->song.name.str, r->song.link.str);
    else
        printf("name: %s\nlink: %s\n", r->album.name.str, r->album.link.str);
    return 0;
}

int main(int argc, char *argv[]) {
    if (argc != 2) {
        printf("%s [search string]\n", argv[0]);
        return 0;
    }
    csn_ctx_t *ctx = csn_init();
    csn_result_t *result = csn_search(ctx, argv[1], 1, 1);

    csn_result_t *head = result;
    while (head) {
        print_entry(head);
        head = head->next;
    }

    csn_free(ctx);
    return 0;
}
