#include "internal.h"

int g_search_options;

/* === STATIC FUNCTIONS === */
/* write curl output to buffer
 */
static uint write_cb(char *in, uint size, uint nmemb, buf_t *out) {
    uint r;
    r = size * nmemb;
    buf_write_mem(out, in, r);
    return r;
}

/* === API PUBLIC FUNCTIONS === */
csn_ctx_t *csn_init(const char *agent) {
    logs("\033[0;33mDEBUG MODE IS ENABLED\n\033[0m");
    logs("\033[0;33mIF YOU DON'T WANT DEBUG INFO, RECOMPILE WITHOUT ENABLE_DEBUG\n\033[0m");

    // check ABI mismatch
    LIBXML_TEST_VERSION;

    csn_ctx_t *ctx = xalloc(sizeof(csn_ctx_t));
    ctx->docbuf = buf_new_size(0);
    ctx->errbuf = buf_new_size(0);
    ctx->error = 0;

    ctx->curl = curl_easy_init();

    // set some curl options
    curl_easy_setopt(ctx->curl, CURLOPT_ERRORBUFFER, ctx->curl_errbuf);
    curl_easy_setopt(ctx->curl, CURLOPT_FOLLOWLOCATION, 1L);

    if (agent == NULL) {
        curl_easy_setopt(ctx->curl, CURLOPT_USERAGENT, DEFAULT_AGENT);
    }
    else {
        curl_easy_setopt(ctx->curl, CURLOPT_USERAGENT, agent);
    }
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
    buf_free(ctx->errbuf);
    curl_easy_cleanup(ctx->curl);

    logs("Done freeing context\n");
    return 0;
}

csn_result_t *csn_search(csn_ctx_t *ctx, const char *str, int options, int limit) {
    if (!ctx || !str) {
        return NULL;
    }

    g_search_options = options;

    // build search url
    char *search_string = curl_easy_escape(ctx->curl, str, strlen(str));
    char temp[2048];
    sprintf(temp, CSN_SEARCH_URL CSN_SEARCH_FMT, search_string, limit);

    curl_easy_setopt(ctx->curl, CURLOPT_URL, temp);
    curl_free(search_string);

    // perform the curl
    logs("Getting data from chiasenhac\n");
    ctx->error = curl_easy_perform(ctx->curl);
    if (!ctx->error) {
        logs("Got data\n");
    }
    else {
        fatalf("Could not get data from `%s`\n", temp);
    }
    return NULL;
}

csn_result_t *csn_fetch_hot(csn_ctx_t *ctx, int type) {
    return NULL;
}

csn_music_info_t *csn_fetch_song_info(csn_ctx_t *ctx, csn_music_t *s) {
    return NULL;
}

csn_music_info_t *csn_fetch_song_info_url(csn_ctx_t *ctx, const char *url) {
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

csn_music_t *csn_music_new() {
    csn_music_t *s = xcalloc(sizeof(csn_music_t));
    return s;
}

csn_album_t *csn_album_new() {
    csn_album_t *a = xcalloc(sizeof(csn_album_t));
    return a;
}

csn_music_info_t *csn_music_info_new() {
    csn_music_info_t *si = xcalloc(sizeof(csn_music_info_t));
    return si;
}

csn_album_info_t *csn_album_info_new() {
    csn_album_info_t *ai = xcalloc(sizeof(csn_album_info_t));
    return ai;
}

csn_artist_t *csn_artist_new() {
   csn_artist_t *a = xcalloc(sizeof(csn_artist_t));
   return a;
}

csn_result_t *csn_result_new(int type) {
    csn_result_t *r = xcalloc(sizeof(csn_result_t));

    if (type == TYPE_MUSIC) {
        r->music = csn_music_new();
    }
    else if (type == TYPE_ALBUM) {
        r->album = csn_album_new();
    }
    else if (type == TYPE_ARTIST) {
        r->artist = csn_artist_new();
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
        if (temp->type == TYPE_MUSIC) {
            csn_music_free(temp->music);
        }
        else if (temp->type == TYPE_ALBUM) {
            csn_album_free(temp->album);
        }
        else if (temp->type == TYPE_ARTIST) {
            csn_artist_free(temp->artist);
        }
        rptr = rptr->next;
        temp = rptr;
    }
}

void csn_album_info_free(csn_album_info_t *ai) {
    if (ai) {
        buf_free(ai->year);

        for (int i = 0; i < ai->num_song; ++i) {
            csn_music_free(ai->song[i]);
        }

        free(ai);
    }
}

static void csn_download_free(csn_download_t *d) {
    for (int i = 0; i < d->num; ++i) {
        buf_free(d->quality[i]);
        buf_free(d->url[i]);
        buf_free(d->size[i]);
    }
}

void csn_music_info_free(csn_music_info_t *si) {
    if (si) {
        csn_download_free(si->download);
        buf_free(si->duration);
        buf_free(si->composer);
        buf_free(si->year);

        csn_album_free(si->album);
        csn_music_free(si->music);

        buf_free(si->lyrics);

        free(si);
    }
}

void csn_music_free(csn_music_t *s) {
    if (s) {
        buf_free(s->title);
        buf_free(s->artist);
        buf_free(s->link);
        buf_free(s->max_quality);

        free(s);
    }
}

void csn_album_free(csn_album_t *a) {
    if (a) {
        buf_free(a->title);
        buf_free(a->link);
        buf_free(a->artist);
        buf_free(a->cover);
        buf_free(a->max_quality);

        free(a);
    }
}

void csn_artist_free(csn_artist_t *a) {
    if (a) {
        buf_free(a->name);
        buf_free(a->link);
        buf_free(a->cover);
        buf_free(a->img);
    }
}
