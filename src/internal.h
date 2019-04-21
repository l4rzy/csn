/*
 * Copyright (C) 2019 l4rzy
 * MIT License
 */

#ifndef _CSN_INTERNAL_H_
#define _CSN_INTERNAL_H_

#include "csn.h"
#include <string.h>
#include <assert.h>
#include <json-c/json_tokener.h>
#include <json-c/json_object_iterator.h>

#ifdef ENABLE_DEBUG
#include <time.h>

clock_t _t_start, _t_end;

#endif

/* chiasenhac.com */
#define CSN_SEARCH_URL          "https://chiasenhac.vn/search/real?"
#define CSN_SEARCH_FMT          "q=%s&type=json&rows=%d&view_all=true"
#define CSN_HOME_URL            "https://chiasenhac.vn"

#ifdef ENABLE_DEBUG
#define logf(fmt, ...) \
    do { \
        fprintf(stderr, "[\033[0;32m%s\033[0m:%d:\033[0;34m%s()\033[0m] "fmt, __FILE__, __LINE__, __func__, \
            __VA_ARGS__); \
    } while (0)

#define logs(str) \
    logf("%s", str)

#else
#define logf(fmt, ...)
#define logs(str)
#endif // ENABLE_DEBUG

#define fatalf(fmt, ...) \
    do { \
        fprintf(stderr, "[FATAL][%s:%d:%s()] "fmt, __FILE__, __LINE__, __func__, \
            __VA_ARGS__); \
        exit(-1); \
    } while (0)

#define fatals(str) \
    fatalf("%s\n", str)

/* for testing */
#define ASSERT(expr) \
    do { \
        assert(expr); \
        printf("\033[0;32m[OK]\033[0m (%s:%d) %s\n", __FILE__, __LINE__, #expr); \
    } while (0);

/* for json parser
 */
typedef struct json_tokener jt_t;
typedef struct json_object jo_t;
typedef struct json_object_iterator jo_iter_t;

#define CSN_S_SEARCH_CAT_SONG       "music"
#define CSN_S_SEARCH_CAT_BEAT       "music_playback"
#define CSN_S_SEARCH_CAT_VIDEO      "video"
#define CSN_S_SEARCH_CAT_ARTIST     "artist"
#define CSN_S_SEARCH_CAT_ALBUM      "album"

/* alloc functions */
void *_xalloc(size_t);
void *_xcalloc(size_t);
void *_xrealloc(void *, size_t);

#define xalloc(x) _xalloc(x)
#define xcalloc(x) _xcalloc(x)
#define xrealloc(x, y) _xrealloc(x, y)

/* buf_t and string functions */
buf_t *buf_new_size(size_t);
buf_t *buf_new_mem(const char *, size_t);
buf_t *buf_new_str(const char *);
buf_t *buf_new_possess(char *);

char *buf_write_char(buf_t *, const char);
char *buf_write_mem(buf_t *, const char *, size_t);
char *buf_write_str(buf_t *, const char *);

char *buf_append_char(buf_t *, const char);
char *buf_append_mem(buf_t *, const char *, size_t);
char *buf_append_str(buf_t *, const char *);

int buf_trim(buf_t *);
int buf_free(buf_t *);

/* parsing functions
 */
csn_result_t *parse_search_result(buf_t *docbuf, int options);

/* function to create and delete result
 */
csn_download_t *csn_download_new();
csn_music_t *csn_music_new();
csn_album_t *csn_album_new();
csn_music_info_t *csn_music_info_new();
csn_album_info_t *csn_album_info_new();
csn_artist_t *csn_artist_new();
csn_result_t *csn_result_new(int type);

void csn_artist_free(csn_artist_t *);

extern int g_search_options;

#endif
