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

/* === xpath functions === */
csn_xpath_t *csn_xpath_parse(const char *str) {
    if (!str) {
        return NULL;
    }
    // WIP
    return NULL;
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

/* === buf_t implementation === */
buf_t *csn_buf_new(size_t size) {
    buf_t *ret = xalloc(sizeof(buf_t));

    ret->str = xcalloc(size+1);
    ret->len = 0;

    return ret;
}

buf_t *csn_buf_from_str(const char *str) {
    int len = strlen(str);
    buf_t *buf = csn_buf_new(len+1);

    memcpy(buf->str, str, len);
    buf->len = len;
    return buf;
}

char *csn_buf_write(buf_t *buf, const char *str) {
    int new_len = strlen(str);
    if (!buf) {
        logs("write to unallocated buf\n");
        buf = csn_buf_new(new_len +1);
    }

    // realloc if new str differs
    if (new_len != buf->len) {
        buf->str = xrealloc(buf->str, new_len+1);
    }
    buf->len = new_len;
    memcpy(buf->str, str, buf->len);
    buf->str[buf->len] = '\0';
    return buf->str;
}

char *csn_buf_append(buf_t *buf, const char *str) {
    int old = buf->len;
    int new_len = strlen(str);
    buf->len += new_len; // new len
    buf->str = xrealloc(buf->str, buf->len+1);

    memcpy(buf->str+old, str, new_len);
    buf->str[buf->len] = '\0';

    return buf->str;
}

int csn_buf_free(buf_t *buf) {
    free(buf->str);
    free(buf);
    return 0;
}

/* ================= */
csn_result_t *csn_result_new(bool is_song) {
    csn_result_t *ret = xalloc(sizeof(csn_result_t));

    ret->next = NULL;
    ret->is_song = is_song;
    return ret;
}

int csn_result_free(csn_result_t *head) {
    csn_result_t *r = head;
    while (r) {
        free(r);
        r = r->next;
    }

    return 0;
}
