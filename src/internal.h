#ifndef _CSN_INTERNAL_H_
#define _CSN_INTERNAL_H_

#include "csn.h"
#include <string.h>
#include <assert.h>
#include <json-c/json_tokener.h>

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


#define CSN_S_SEARCH_ARTIST                     "artist"
#define CSN_S_SEARCH_SONG                       ""
#define CSN_S_SEARCH_COMPOSER                   "composer"
#define CSN_S_SEARCH_ALBUM                      "album"
#define CSN_S_SEARCH_LYRICS                     "lyric"

#define CSN_S_SEARCH_SORT_MOST_LOVED            ""
#define CSN_S_SEARCH_SORT_BEST_QUALITY          "quality"
#define CSN_S_SEARCH_SORT_LATEST                "time"

#define CSN_S_SEARCH_CATEGORY_MUSIC             "music"
#define CSN_S_SEARCH_CATEGORY_BEAT              "playback"
#define CSN_S_SEARCH_CATEGORY_VIDEO             "video"

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

/* function to create result
 */
csn_download_t *csn_download_new();
csn_music_t *csn_music_new();
csn_album_t *csn_album_new();
csn_music_info_t *csn_music_info_new();
csn_album_info_t *csn_album_info_new();
csn_artist_t *csn_artist_new();
csn_result_t *csn_result_new(int type);

/* other utilities
 */
char *build_search_url(const char *, int, int);

extern int g_search_options;

#endif
