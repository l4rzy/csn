#include "internal.h"

int csn_print_tidy_error(csn_ctx_t *ctx) {
    if (!ctx->tidy_errbuf.bp) {
        return 0;
    }
    fprintf(stderr, "%s\n", ctx->tidy_errbuf.bp); /* show errors */
    return 0;
}
