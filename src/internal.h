#ifndef _CSN_INTERNAL_H_
#define _CSN_INTERNAL_H_

#include "csn.h"

/* chiasenhac.com */
#define CSN_SEARCH_URL "http://search.chiasenhac.vn/search.php"

#ifdef ENABLE_DEBUG
#define logf(fmt, ...) \
    do { \
        fprintf(stderr, "[%s:%d:%s()] "fmt, __FILE__, __LINE__, __func__, \
            __VA_ARGS__); \
    } while (0)

#define logs(str) \
    logf("%s\n", str)

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


#define CSN_S_SEARCH_ARTIST                     "artist"
#define CSN_S_SEARCH_SONG                       ""
#define CSN_S_SEARCH_COMPOSER                   "composer"
#define CSN_S_SEARCH_ALBUM                      "album"
#define CSN_S_SEARCH_LYRICS                     "lyric"

#define CSN_S_SEARCH_MOST_LOVED_FIRST           ""
#define CSN_S_SEARCH_BEST_QUALITY_FIRST         "quality"
#define CSN_S_SEARCH_LASTEST_FIRST              "time"

#define CSN_S_SEARCH_CATEGORY_MUSIC             "music"
#define CSN_S_SEARCH_CATEGORY_BEAT              "playback"
#define CSN_S_SEARCH_CATEGORY_VIDEO             "video"

typedef struct _csn_xpath_t {
    char *name;
    int index;
    struct _csn_xpath_t *next;
} csn_xpath_t;

void *_xalloc(size_t);
void *_xcalloc(size_t);
void *_xrealloc(void *, size_t);

#define xalloc(x) _xalloc(x)
#define xcalloc(x) _xcalloc(x)
#define xrealloc(x, y) _xrealloc(x, y)

buf_t *csn_buf_new(size_t);
buf_t *csn_buf_from_str(const char *);
char *csn_buf_write(buf_t *, const char *);
char *csn_buf_append(buf_t *, const char *);
int csn_buf_free(buf_t *);

/* parsing functions
 */
csn_result_t *parse_search_result(TidyDoc, TidyNode, int);

#endif
