#ifndef _CSN_INTERNAL_H_
#define _CSN_INTERNAL_H_

#include "csn.h"

#ifdef ENABLE_DEBUG
#include <time.h>

clock_t _t_start, _t_end;

#endif

/* chiasenhac.com */
#define CSN_SEARCH_URL          "http://search.chiasenhac.vn/search.php"
#define CSN_HOME_URL            "http://old.chiasenhac.vn"

#define CSN_HOT_VIDEOS_XPATH    "/html/body/div[1]/div/div[3]/div/div[1]/div/div[5]/div/div[1]/div[2]"
#define CSN_HOT_VN_XPATH        "/html/body/div[1]/div/div[3]/div/div[1]/div/div[5]/div/div[2]/div[2]"
#define CSN_HOT_USUK_XPATH      "/html/body/div[1]/div/div[3]/div/div[4]/div/div[5]/div/div[2]/div[2]"
#define CSN_SEARCH_XPATH        "/html/body/div[1]/div/div[3]/div/div/div[5]/div[1]/div[3]/div[2]/table"

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
        printf("\033[0;32m[OK]\033[0m %s\n", #expr); \
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

/* for xpath navigation in DOM
 */
typedef struct _csn_xpath_t {
    buf_t *tag;
    bool is_root;
    int index;
    struct _csn_xpath_t *next;
} csn_xpath_t;

/* a queue of tidynodes
 */
typedef struct _csn_node_t {
    TidyNode tnode;
    struct _csn_node_t *next;
} csn_node_t;

typedef struct _csn_queue_t {
    csn_node_t *head;
    csn_node_t *tail;
    int len;
} csn_queue_t;

/* xpath functions */
csn_xpath_t *csn_xpath_parse(const char *);
csn_xpath_t *csn_xpath_new();
TidyNode csn_xpath_traverse(TidyNode, csn_xpath_t *);
void csn_xpath_free(csn_xpath_t *);

/* queue functions */
csn_node_t *csn_node_new();
csn_queue_t *csn_queue_new();
TidyNode csn_enqueue(csn_queue_t *, TidyNode);
TidyNode csn_dequeue(csn_queue_t *);
void csn_queue_free(csn_queue_t *);

/* alloc functions */
void *_xalloc(size_t);
void *_xcalloc(size_t);
void *_xrealloc(void *, size_t);

#define xalloc(x) _xalloc(x)
#define xcalloc(x) _xcalloc(x)
#define xrealloc(x, y) _xrealloc(x, y)

/* buf_t and string functions */
buf_t *csn_buf_new(size_t);
buf_t *csn_buf_from_str(const char *);
buf_t *csn_buf_possess(char *);
char *csn_buf_write(buf_t *, const char *);
char *csn_buf_write_char(buf_t *, const char);
char *csn_buf_append(buf_t *, const char *);
char *csn_buf_append_char(buf_t *, const char);
int csn_buf_trim(buf_t *);
int csn_buf_free(buf_t *);

/* parsing functions
 */
csn_result_t *parse_song_search_result(TidyDoc);

/* function to create result
 */
csn_download_t *csn_download_new();
csn_song_t *csn_song_new(int type);
csn_album_t *csn_album_new();
csn_song_info_t *csn_song_info_new();
csn_album_info_t *csn_album_info_new();
csn_result_t *csn_result_new(bool is_song);

/* other utilities
 */
char *build_search_url(const char *, int, int);

extern int g_search_options;

#endif
