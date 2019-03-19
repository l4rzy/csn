#include "csn.h"

int main(int argc, char *argv[]) {
    if (argc != 2) {
        printf("%s [search string]\n", argv[0]);
        return 0;
    }
    csn_ctx_t *ctx = csn_init();
    csn_search(ctx, argv[1], 1, 1);
    csn_free(ctx);

    return 0;
}
