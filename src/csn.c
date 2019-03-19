#include "internal.h"
#include "csn.h"

/* static vars
 */
static csn_result_t *head;

/* write curl output to buffer that's ready to be used by tidy
 */
static uint write_cb(char *in, uint size, uint nmemb, TidyBuffer *out) {
    uint r;
    r = size * nmemb;
    tidyBufAppend(out, in, r);
    return r;
}

csn_result_t *parse_search_result(TidyDoc doc, TidyNode tnod, int indent)
{
    TidyNode child;
    for (child = tidyGetChild(tnod); child; child = tidyGetNext(child) ) {
        ctmbstr name = tidyNodeGetName(child);
        if (name && !strcmp(name, "table")) {
            logs("body here\n");
        }
        // else {
        //     /* if it doesn't have a name, then it's probably text, cdata, etc... */
        //     TidyBuffer buf;
        //     tidyBufInit(&buf);
        //     tidyNodeGetText(doc, child, &buf);
        //     //printf("%*.*s\n", indent, indent, buf.bp?(char *)buf.bp:"");
        //     tidyBufFree(&buf);
        // }
        parse_search_result(doc, child, indent + 4); /* recursive */
    }
    return NULL;
}

static char *build_url(const char *str, int options, int limit) {
    char temp[1024]; //TODO: fix this
    const char *search_fmt = CSN_SEARCH_URL"?s=%s&mode=%s&order=%s&cat=%s";

    sprintf(temp, search_fmt, str, "", "", "music");
    logf("%s\n", temp);
    return strdup(temp);
}

csn_ctx_t *csn_init() {
    csn_ctx_t *ctx = xalloc(sizeof(csn_ctx_t));
    bzero(&ctx->docbuf, sizeof(ctx->docbuf));
    bzero(&ctx->tidy_errbuf, sizeof(ctx->tidy_errbuf));

    ctx->curl = curl_easy_init();
    // set some curl options
    curl_easy_setopt(ctx->curl, CURLOPT_ERRORBUFFER, ctx->curl_errbuf);
    curl_easy_setopt(ctx->curl, CURLOPT_NOPROGRESS, 1L);
    curl_easy_setopt(ctx->curl, CURLOPT_VERBOSE, 0L);
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
    tidyBufFree(&ctx->docbuf);
    tidyBufFree(&ctx->tidy_errbuf);
    tidyRelease(ctx->tdoc);

    logs("Done freeing context\n");
    return 0;
}

csn_result_t *csn_search(csn_ctx_t *ctx, const char *str, int options, int limit) {
    csn_result_t *ret = malloc(sizeof(csn_result_t));
    if (!ret) {
        fatals("Could not allocate memory!\n");
    }
    // build search url
    char *search_string = curl_easy_escape(ctx->curl, str, strlen(str));
    char *surl = build_url(search_string, options, limit);

    curl_easy_setopt(ctx->curl, CURLOPT_URL, surl);
    curl_free(search_string);
    free(surl);
    ctx->err = curl_easy_perform(ctx->curl);
    if (!ctx->err) {
        ctx->err = tidyParseBuffer(ctx->tdoc, &ctx->docbuf); /* parse the input */
        if (ctx->err >= 0) {
            ctx->err = tidyCleanAndRepair(ctx->tdoc); /* fix any problems */
            if (ctx->err >= 0) {
                ctx->err = tidyRunDiagnostics(ctx->tdoc); /* load tidy error buffer */
                if (ctx->err >= 0) {
                    return parse_search_result(ctx->tdoc, tidyGetRoot(ctx->tdoc), 0); /* walk the tree */
                }
            }
        }
    }
    else {
        fatalf("Could not get data from `%s`\n", surl);
    }
    return ret;
}
