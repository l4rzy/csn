#include "internal.h"

int g_search_options;

/* === STATIC FUNCTIONS === */
/* write curl output to buffer that's ready to be used by tidy
 */
static uint write_cb(char *in, uint size, uint nmemb, TidyBuffer *out) {
    uint r;
    r = size * nmemb;
    tidyBufAppend(out, in, r);
    return r;
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
                    if (g_search_options & SEARCH_ALBUM) {
                        return NULL; // TODO:
                    }
                    else {
                        return parse_song_search_result(ctx->tdoc); /* walk the tree */
                    }
                }
            }
        }
    }
    else {
        fatalf("Could not get data from `%s`\n", surl);
    }
    return NULL;
}

csn_result_t *csn_fetch_hot(csn_ctx_t *ctx, int type, int limit) {
    return NULL;
}

csn_song_info_t *csn_fetch_song_info(csn_ctx_t *ctx, csn_song_t *s) {
    return NULL;
}

csn_song_info_t *csn_fetch_song_info_url(csn_ctx_t *ctx, const char *url) {
    return NULL;
}

csn_album_info_t *csn_fetch_album_info(csn_ctx_t *ctx, csn_album_t *a) {
    return NULL;
}

csn_album_info_t *csn_fetch_album_info_url(csn_ctx_t *ctx, const char *url) {
    return NULL;
}

/* function to create csn objects
 */
csn_download_t *csn_download_new() {
    csn_download_t *dl = xcalloc(sizeof(csn_download_t));
    return dl;
}

csn_song_t *csn_song_new(int type) {
    csn_song_t *s = xcalloc(sizeof(csn_song_t));
    s->type = type;

    return s;
}

csn_album_t *csn_album_new() {
    csn_album_t *a = xcalloc(sizeof(csn_album_t));
    return a;
}

csn_song_info_t *csn_song_info_new() {
    csn_song_info_t *si = xcalloc(sizeof(csn_song_info_t));
    return si;
}

csn_album_info_t *csn_album_info_new() {
    csn_album_info_t *ai = xcalloc(sizeof(csn_album_info_t));
    return ai;
}

csn_result_t *csn_result_new(bool is_song) {
    csn_result_t *r = xcalloc(sizeof(csn_result_t));

    r->is_song = is_song;
    if (r->is_song) {
        r->song = csn_song_new(CSN_TYPE_SONG);
    }
    else {
        r->album = csn_album_new();
    }
    return r;
}

/* functions to free memory
 */
void csn_result_free(csn_result_t *r) {
    csn_result_t *rptr;
    csn_result_t *temp;

    rptr = r;
    temp = r;

    // free a linked list
    while (temp) {
        logf("Freeing result at %p\n", temp);
        if (temp->is_song) {
            csn_song_free(temp->song);
        } else {
            csn_album_free(temp->album);
        }
        rptr = rptr->next;
        temp = rptr;
    }
}

void csn_album_info_free(csn_album_info_t *ai) {
    if (ai) {
        csn_buf_free(ai->title);
        csn_buf_free(ai->artist);
        csn_buf_free(ai->year);

        for (int i = 0; i < ai->num_song; ++i) {
            csn_song_free(ai->song[i]);
        }

        free(ai);
    }
}

static void csn_download_free(csn_download_t *d) {
    if (d) {
        csn_buf_free(d->quality);
        csn_buf_free(d->url);
        csn_buf_free(d->size);

        free(d);
    }
}

void csn_song_info_free(csn_song_info_t *si) {
    if (si) {
        csn_buf_free(si->title);
        csn_buf_free(si->artist);
        csn_buf_free(si->composer);
        csn_buf_free(si->year);

        csn_album_free(si->album);

        csn_buf_free(si->lyrics);

        for (int i = 0; i < si->num_download; ++i) {
            csn_download_free(si->download[i]);
        }

        free(si);
    }
}

void csn_song_free(csn_song_t *s) {
    if (s) {
        csn_buf_free(s->name);
        csn_buf_free(s->link);
        csn_buf_free(s->duration);
        csn_buf_free(s->max_quality);

        free(s);
    }
}

void csn_album_free(csn_album_t *a) {
    if (a) {
        csn_buf_free(a->name);
        csn_buf_free(a->link);
        csn_buf_free(a->cover);
        csn_buf_free(a->max_quality);

        free(a);
    }
}
