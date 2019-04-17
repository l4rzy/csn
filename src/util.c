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

/* builds search url from options, currently lthe limit options is disable
 * but in the future, maybe we'll make more than one request to get some
 * further pages in order to get the number of results as user wanted
 */
char *build_search_url(const char *str, int options, int limit) {
#ifdef ENABLE_DEBUG
    _t_start = clock();
#endif
    g_search_options = options;

    buf_t *search_type = csn_buf_new(0);
    buf_t *search_sort = csn_buf_new(0);
    buf_t *search_cat  = csn_buf_new(0);

    /* failed to do it the more rational way due to the limitation of
     * C macro system
     */
#define TYPE_CASE(op, s) \
    if (options & op) { \
        csn_buf_write(search_type, s); \
    }

#define SORT_CASE(op, s) \
    if (options & op) { \
        csn_buf_write(search_sort, s); \
    }

#define CAT_CASE(op, s) \
    if (options & op) { \
        csn_buf_write(search_cat, s); \
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
    csn_buf_free(search_type);
    csn_buf_free(search_sort);
    csn_buf_free(search_cat);
#ifdef ENABLE_DEBUG
    _t_end = clock();
    logf("Took %ld to complete\n", _t_end - _t_start);
#endif
    return strdup(temp);
}

/* TODO: rewriting this the right way
 */
char *old_csn_url(const char *surl) {
/* http://beta.chiasenhac.vn/mp3/vietnam/v-pop/mua-hoa-bo-lai~thu-phuong~ts3vqd5rq2emha.html
 * => http://chiasenhac.vn/mp3/vietnam/v-pop/mua-hoa-bo-lai~thu-phuong~ts3vqd5rq2emha.html
 */
    char *ret = strdup(surl);
    int len = strlen(ret);
    memmove(ret + 4, ret + 5, len - 5 + 1);
    memcpy(ret + 11, ret + 12, len - 12 + 1);
    memcpy(ret + 7, "old.", 4);

    ret = xrealloc(ret, len - 2);
    return ret;
}

/* === queue implementation === */
csn_node_t *csn_node_new() {
    csn_node_t *node = xalloc(sizeof(csn_node_t));
    return node;
}

csn_queue_t *csn_queue_new() {
    csn_queue_t *queue = xalloc(sizeof(csn_queue_t));
    return queue;
}

TidyNode csn_enqueue(csn_queue_t *queue, TidyNode n) {
    csn_node_t *new_node = csn_node_new();
    new_node->tnode = n;
    new_node->next = NULL;

    // queue is empty
    if (!queue->head) {
        queue->head = new_node;
        queue->tail = new_node;
    }
    // since we saved the tail node, insertion time complexity is
    // O(1) instead of O(n)
    else {
        queue->tail->next = new_node;
        queue->tail = new_node;
    }
    ++queue->len;
    return n;
}

TidyNode csn_dequeue(csn_queue_t *q) {
    if (!q) {
        return NULL;
    }
    TidyNode save = q->head->tnode;
    csn_node_t *new_head = q->head->next;
    free(q->head);
    q->head = new_head;
    --q->len;
    // return new head
    return save;
}

void csn_queue_free(csn_queue_t *q) {
    csn_node_t *ptr = q->head;
    csn_node_t *tmp;
    while (1) {
        if (!ptr) {
            free(q);
            break;
        }
        tmp = ptr->next;
        free(ptr);
        ptr = tmp;
    }
}

#ifdef ENABLE_DEBUG
void csn_queue_print(csn_queue_t *q) {
    if (!q) {
        logs("Empty queue!\n");
    }

    int count = 0;
    csn_node_t *ptr = q->head;
    logf("Queue at %p has %d node(s)\n", q, q->len);
    while (ptr != q->tail) {
        logf("node #%d at %p", count++, ptr);
        ptr = ptr->next;
    }
    // print the tail
    logf("Tail is at %p\n", q->tail);
}
#endif
