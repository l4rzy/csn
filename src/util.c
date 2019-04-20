#include "internal.h"
#include <ctype.h>

void *_xalloc(size_t size) {
    void *ret = malloc(size);
    if (!ret) {
        fatals("Could not allocate memory\n");
    }
    return ret;
}

void *_xcalloc(size_t size) {
    void *ret = calloc(size, 1);

    if (!ret) {
        fatals("Could not allocate memory\n");
    }
    return ret;
}

void *_xrealloc(void *ptr, size_t new_size) {
    ptr = realloc(ptr, new_size);
    if (!ptr) {
        fatals("Could not allocate memory\n");
    }
    return ptr;
}

char *build_search_url(const char *str, int options, int limit) {
    g_search_options = options;

    char temp[2048];
    sprintf(temp, CSN_SEARCH_URL, str);
    return strdup(temp);
}

/* builds search url from options, currently lthe limit options is disable
 * but in the future, maybe we'll make more than one request to get some
 * further pages in order to get the number of results as user wanted
 */
char *_build_search_url(const char *str, int options, int limit) {
#ifdef ENABLE_DEBUG
    _t_start = clock();
#endif
    g_search_options = options;

    buf_t *search_type = buf_new_size(0);
    buf_t *search_sort = buf_new_size(0);
    buf_t *search_cat  = buf_new_size(0);

    /* failed to do it the more rational way due to the limitation of
     * C macro system
     */
#define TYPE_CASE(op, s) \
    if (options & op) { \
        buf_write_str(search_type, s); \
    }

#define SORT_CASE(op, s) \
    if (options & op) { \
        buf_write_str(search_sort, s); \
    }

#define CAT_CASE(op, s) \
    if (options & op) { \
        buf_write_str(search_cat, s); \
    }

    TYPE_CASE(SEARCH_ARTIST, CSN_S_SEARCH_ARTIST);
    TYPE_CASE(SEARCH_SONG, CSN_S_SEARCH_SONG);
    TYPE_CASE(SEARCH_COMPOSER, CSN_S_SEARCH_COMPOSER);
    TYPE_CASE(SEARCH_ALBUM, CSN_S_SEARCH_ALBUM);
    TYPE_CASE(SEARCH_LYRICS, CSN_S_SEARCH_LYRICS);

    SORT_CASE(SEARCH_SORT_MOST_LOVED, CSN_S_SEARCH_SORT_MOST_LOVED);
    SORT_CASE(SEARCH_SORT_BEST_QUALITY, CSN_S_SEARCH_SORT_BEST_QUALITY);
    SORT_CASE(SEARCH_SORT_LATEST, CSN_S_SEARCH_SORT_LATEST);

    CAT_CASE(SEARCH_CATEGORY_MUSIC, CSN_S_SEARCH_CATEGORY_MUSIC);
    CAT_CASE(SEARCH_CATEGORY_BEAT, CSN_S_SEARCH_CATEGORY_BEAT);
    CAT_CASE(SEARCH_CATEGORY_VIDEO, CSN_S_SEARCH_CATEGORY_VIDEO);

#undef TYPE_CASE
#undef SORT_CASE
#undef CAT_CASE

    logf("search options:\ntype: %s\nsort: %s\ncat: %s\n", search_type->str, search_sort->str, search_cat->str);

    char temp[1024]; //TODO: fix this
    const char *search_fmt = CSN_SEARCH_URL"?s=%s&mode=%s&order=%s&cat=%s";

    sprintf(temp, search_fmt, str, search_type->str, search_sort->str, search_cat->str);
    logf("%s\n", temp);

    /* free stuff
     */
    buf_free(search_type);
    buf_free(search_sort);
    buf_free(search_cat);
#ifdef ENABLE_DEBUG
    _t_end = clock();
    logf("Took %ld to complete\n", _t_end - _t_start);
#endif
    return strdup(temp);
}
