#ifndef _CSN_H_
#define _CSN_H_
#ifdef __cplusplus
extern "C" {
#endif

#include <stdio.h>
#include <stddef.h>
#include <tidy.h>
#include <tidybuffio.h>
#include <curl/curl.h>
#include <stdbool.h>

enum {
    CSN_TYPE_SONG,
    CSN_TYPE_BEAT,
    CSN_TYPE_VIDEO,
};

enum {
    CSN_HOT_VN,
    CSN_HOT_USUK,
    CSN_HOT_CLIP
};

enum {
    SEARCH_ARTIST               = 1 << 1,
    SEARCH_SONG                 = 1 << 2,
    SEARCH_COMPOSER             = 1 << 3,
    SEARCH_ALBUM                = 1 << 4,
    SEARCH_LYRICS               = 1 << 5,

    SEARCH_SORT_MOST_LOVED      = 1 << 6,
    SEARCH_SORT_BEST_QUALITY    = 1 << 7,
    SEARCH_SORT_LATEST          = 1 << 8,

    SEARCH_CATEGORY_MUSIC       = 1 << 9,
    SEARCH_CATEGORY_BEAT        = 1 << 10,
    SEARCH_CATEGORY_VIDEO       = 1 << 11
};

typedef struct _buf_t {
    char *str;
    int len;
} buf_t;

typedef struct _csn_ctx_t {
    CURL *curl;
    char curl_errbuf[CURL_ERROR_SIZE];
    TidyDoc tdoc;
    TidyBuffer docbuf;
    TidyBuffer tidy_errbuf;
    int err;
} csn_ctx_t;

typedef struct _csn_download_t {
    buf_t *quality;
    buf_t *url;
    buf_t *size;
} csn_download_t;

typedef struct _csn_song_t {
    int type; // either SONG BEAT or VIDEO
    buf_t *title;
    buf_t *artist;
    buf_t *link;
    buf_t *duration;
    buf_t *max_quality;
    buf_t *download_count;
} csn_song_t;

typedef struct _csn_album_t {
    buf_t *title;
    buf_t *link;
    buf_t *cover; // the small album cover
    buf_t *max_quality;
} csn_album_t;

typedef struct _csn_song_info_t {
    int type; // either SONG BEAT or VIDEO
    buf_t *title;
    buf_t *artist;
    buf_t *composer;
    buf_t *year;
    csn_album_t *album;
    buf_t *lyrics;
    csn_download_t **download;
    int num_download;
} csn_song_info_t;

typedef struct _csn_album_info_t {
    buf_t *title;
    buf_t *artist;
    buf_t *year;
    csn_song_t **song; // an array of pointers to csn_song_t
    int num_song;
} csn_album_info_t;

/* result is the metadata struct for csn
 */
typedef struct _csn_result_t {
    union {
        csn_song_t *song; // both song and beat and video
        csn_album_t *album;
    };
    bool is_song; // true for song, false for album
    struct _csn_result_t *next; // this is a recursive structure
} csn_result_t;

/* constructor, destructor
 */
csn_ctx_t *csn_init();
int csn_free(csn_ctx_t *);

/* API functions
 */
csn_result_t *csn_search(csn_ctx_t *, const char *, int, int);
csn_result_t *csn_fetch_hot(csn_ctx_t *, int, int);
csn_song_info_t *csn_fetch_song_info_url(csn_ctx_t *, const char *);
csn_album_info_t *csn_fetch_album_info_url(csn_ctx_t *, const char *);
csn_song_info_t *csn_fetch_song_info(csn_ctx_t *, csn_song_t *);
csn_album_info_t *csn_fetch_album_info(csn_ctx_t *, csn_album_t *);
csn_song_info_t **csn_batch_fetch_song_info(csn_ctx_t *, csn_album_info_t *);

/* functions for free memory
 */
void csn_result_free(csn_result_t *);
void csn_album_info_free(csn_album_info_t *);
void csn_song_info_free(csn_song_info_t *);
void csn_song_free(csn_song_t *);
void csn_album_free(csn_album_t *);

#ifdef __cplusplus
}
#endif

#endif
