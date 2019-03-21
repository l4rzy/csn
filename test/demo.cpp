#include "csn.h"

int main(int argc, char *argv[]) {
    if (argc != 2) {
        return -1;
    }
    csn_ctx_t *ctx = csn_init();
    csn_result_t *result = csn_search(ctx, argv[1], 1, 1);
    csn_free(ctx);
    return 0;
}
