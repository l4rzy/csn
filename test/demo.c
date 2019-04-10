#include "csn.h"

int print_entry(csn_result_t *r) {
    if (r->is_song) {
        printf("link: %s\ntitle: %s\nartist: %s\nlength: %s\nmax_qual: %s\ndownloads: %s\n",
            r->song->link->str,
            r->song->title->str,
            r->song->artist->str,
            r->song->duration->str,
            r->song->max_quality->str,
            r->song->download_count->str
);
    }
    return 0;
}

int main(int argc, char *argv[]) {
    if (argc != 2) {
        printf("%s [search string]\n", argv[0]);
        return 0;
    }
    csn_ctx_t *ctx = csn_init();
    csn_result_t *result = csn_search(ctx, argv[1],
            SEARCH_SONG | SEARCH_SORT_BEST_QUALITY | SEARCH_CATEGORY_MUSIC, 1);

    csn_result_t *head = result;
    while (head) {
        print_entry(head);
        puts("=====");
        head = head->next;
    }

    csn_result_free(result);
    csn_free(ctx);
    return 0;
}
