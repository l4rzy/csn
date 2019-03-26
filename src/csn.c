#include "internal.h"

/* === STATIC FUNCTIONS === */
/* write curl output to buffer that's ready to be used by tidy
 */
static uint write_cb(char *in, uint size, uint nmemb, TidyBuffer *out) {
    uint r;
    r = size * nmemb;
    tidyBufAppend(out, in, r);
    return r;
}

static char *build_search_url(const char *str, int options, int limit) {
    char temp[1024]; //TODO: fix this
    const char *search_fmt = CSN_SEARCH_URL"?s=%s&mode=%s&order=%s&cat=%s";

    sprintf(temp, search_fmt, str, "", "", "music");
    logf("%s\n", temp);
    return strdup(temp);
}

/* === API PUBLIC FUNCTIONS === */
csn_ctx_t *csn_init() {
    logs("\033[0;33mDEBUG MODE IS ENABLED\n\033[0m");
    logs("\033[0;33mIF YOU DON'T WANT DEBUG INFO, RECOMPILE WITHOUT ENABLE_DEBUG\n\033[0m");
    csn_ctx_t *ctx = xalloc(sizeof(csn_ctx_t));
    bzero(&ctx->docbuf, sizeof(ctx->docbuf));
    bzero(&ctx->tidy_errbuf, sizeof(ctx->tidy_errbuf));

    ctx->curl = curl_easy_init();
    // set some curl options
    curl_easy_setopt(ctx->curl, CURLOPT_ERRORBUFFER, ctx->curl_errbuf);
    curl_easy_setopt(ctx->curl, CURLOPT_NOPROGRESS, 1L);
#ifdef ENABLE_DEBUG
    curl_easy_setopt(ctx->curl, CURLOPT_VERBOSE, 1L);
#else
    curl_easy_setopt(ctx->curl, CURLOPT_VERBOSE, 0L);
#endif

    curl_easy_setopt(ctx->curl, CURLOPT_WRITEFUNCTION, write_cb);

    ctx->tdoc = tidyCreate();
    tidyOptSetBool(ctx->tdoc, TidyForceOutput, yes); /* try harder */
    tidyOptSetInt(ctx->tdoc, TidyWrapLen, 4096);
    tidySetErrorBuffer(ctx->tdoc, &ctx->tidy_errbuf);
    tidyBufInit(&ctx->docbuf);

    curl_easy_setopt(ctx->curl, CURLOPT_WRITEDATA, &ctx->docbuf);
    logf("Done initializing context at %p\n", ctx);
    return ctx;
}

int csn_free(csn_ctx_t *ctx) {
    curl_easy_cleanup(ctx->curl);
    if (&ctx->docbuf)
        tidyBufFree(&ctx->docbuf);
    if (&ctx->tidy_errbuf)
        tidyBufFree(&ctx->tidy_errbuf);
    tidyRelease(ctx->tdoc);

    logs("Done freeing context\n");
    return 0;
}

csn_result_t *csn_search(csn_ctx_t *ctx, const char *str, int options, int limit) {
    csn_result_t *ret = xalloc(sizeof(csn_result_t));
    // build search url
    char *search_string = curl_easy_escape(ctx->curl, str, strlen(str));
    char *surl = build_search_url(search_string, options, limit);

    curl_easy_setopt(ctx->curl, CURLOPT_URL, surl);
    curl_free(search_string);
    free(surl);
    // perform the curl
    logs("Getting data from chiasenhac\n");
    ctx->err = curl_easy_perform(ctx->curl);
    if (!ctx->err) {
        ctx->err = tidyParseBuffer(ctx->tdoc, &ctx->docbuf); /* parse the input */
        if (ctx->err >= 0) {
            ctx->err = tidyCleanAndRepair(ctx->tdoc); /* fix any problems */
            if (ctx->err >= 0) {
                ctx->err = tidyRunDiagnostics(ctx->tdoc); /* load tidy error buffer */
                if (ctx->err >= 0) {
                    return parse_search_result(ctx->tdoc); /* walk the tree */
                }
            }
        }
    }
    else {
        fatalf("Could not get data from `%s`\n", surl);
    }
    return ret;
}

csn_result_t *csn_fetch_hot(csn_ctx_t *ctx, int type, int limit) {
    return NULL;
}

csn_song_info_t *csn_fetch_song_info(csn_ctx_t *ctx, csn_song_t *s) {
    return NULL;
}

csn_album_info_t *csn_fetch_album_info(csn_ctx_t *ctx, csn_album_t *a) {
    return NULL;
}
