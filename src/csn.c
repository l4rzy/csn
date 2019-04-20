#include "internal.h"

int g_search_options;

/* === STATIC FUNCTIONS === */
/* write curl output to buffer that's ready to be used by tidy
 */
static uint write_cb(char *in, uint size, uint nmemb, buf_t *out) {
    uint r;
    r = size * nmemb;
    buf_write_mem(out, in, r);
    return r;
}

/* === API PUBLIC FUNCTIONS === */
csn_ctx_t *csn_init() {
    logs("\033[0;33mDEBUG MODE IS ENABLED\n\033[0m");
    logs("\033[0;33mIF YOU DON'T WANT DEBUG INFO, RECOMPILE WITHOUT ENABLE_DEBUG\n\033[0m");

    // check ABI mismatch
    LIBXML_TEST_VERSION

    csn_ctx_t *ctx = xalloc(sizeof(csn_ctx_t));
    ctx->docbuf = buf_new_size(0);
    ctx->parser_ctx = htmlCreateMemoryParserCtxt(ctx->docbuf->str, ctx->docbuf->len);

    ctx->curl = curl_easy_init();
    // set some curl options
    curl_easy_setopt(ctx->curl, CURLOPT_ERRORBUFFER, ctx->curl_errbuf);
    // TODO: custom user agent
    curl_easy_setopt(ctx->curl, CURLOPT_USERAGENT, "Mozilla/5.0 (X11; Linux x86_64; rv:66.0) Gecko/20100101 Firefox/66.0");
#ifdef ENABLE_DEBUG
    curl_easy_setopt(ctx->curl, CURLOPT_NOPROGRESS, 0L);
    curl_easy_setopt(ctx->curl, CURLOPT_VERBOSE, 1L);
#else
    curl_easy_setopt(ctx->curl, CURLOPT_NOPROGRESS, 1L);
    curl_easy_setopt(ctx->curl, CURLOPT_VERBOSE, 0L);
#endif

    curl_easy_setopt(ctx->curl, CURLOPT_WRITEFUNCTION, write_cb);
    curl_easy_setopt(ctx->curl, CURLOPT_WRITEDATA, ctx->docbuf);
    logf("Done initializing context at %p\n", ctx);
    return ctx;
}

int csn_free(csn_ctx_t *ctx) {
    buf_free(ctx->docbuf);
    curl_easy_cleanup(ctx->curl);
    logs("Done freeing context\n");
    return 0;
}

csn_result_t *csn_search(csn_ctx_t *ctx, const char *str, int options, int limit) {
    // build search url
    char *search_string = curl_easy_escape(ctx->curl, str, strlen(str));
    char *surl = build_search_url(search_string, options, limit);

    curl_easy_setopt(ctx->curl, CURLOPT_URL, surl);
    curl_free(search_string);

    // perform the curl
    logs("Getting data from chiasenhac\n");
    ctx->err = curl_easy_perform(ctx->curl);
    if (!ctx->err) {
        ctx->docptr = htmlCtxtReadMemory(ctx->parser_ctx, ctx->docbuf->str,
            ctx->docbuf->len, NULL, surl, 0);
        htmlDocContentDumpOutput(ctx->docbuf->str, ctx->docptr, NULL);
        // parsing to tree and get data
    }
    else {
        fatalf("Could not get data from `%s`\n", surl);
    }
    free(surl);
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
        buf_free(ai->title);
        buf_free(ai->artist);
        buf_free(ai->year);

        for (int i = 0; i < ai->num_song; ++i) {
            csn_song_free(ai->song[i]);
        }

        free(ai);
    }
}

static void csn_download_free(csn_download_t *d) {
    if (d) {
        buf_free(d->quality);
        buf_free(d->url);
        buf_free(d->size);

        free(d);
    }
}

void csn_song_info_free(csn_song_info_t *si) {
    if (si) {
        buf_free(si->title);
        buf_free(si->artist);
        buf_free(si->composer);
        buf_free(si->year);

        csn_album_free(si->album);

        buf_free(si->lyrics);

        for (int i = 0; i < si->num_download; ++i) {
            csn_download_free(si->download[i]);
        }

        free(si);
    }
}

void csn_song_free(csn_song_t *s) {
    if (s) {
        buf_free(s->title);
        buf_free(s->artist);
        buf_free(s->link);
        buf_free(s->duration);
        buf_free(s->max_quality);
        buf_free(s->download_count);

        free(s);
    }
}

void csn_album_free(csn_album_t *a) {
    if (a) {
        buf_free(a->title);
        buf_free(a->link);
        buf_free(a->cover);
        buf_free(a->max_quality);

        free(a);
    }
}
