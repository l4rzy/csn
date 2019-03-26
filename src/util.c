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

/* parse xpath into linked list, this support a subset of xpath
 * and for use locally, so I guess this implementation is ok :)
 */
csn_xpath_t *csn_xpath_parse(const char *str) {
    if (!str) {
        return NULL;
    }

    csn_xpath_t *root = NULL;
    csn_xpath_t *xptr;
    const char *ptr = str;
    char buffer[1024]; // assume that it's no bigger than 1023
    int buflen = 0;

    while (1) {
        if (*ptr == '\0' ) {
            break;
        }
        if (*ptr == '/') {
            // create new node, if there is no root, set it as root
            if (!root) {
                logs("Creating root node\n");
                root = csn_xpath_new();
                root->is_root = true;
                root->next = NULL;

                xptr = root;

                // then continue
                continue;
            }
            else {
                ++ptr;
                // consume until `/` or `[`
                logf("Start consuming from %p\n", ptr);
                while (*ptr != '/' &&
                       *ptr != '[' &&
                       *ptr != ']' &&
                       *ptr != '\0') {
                    buffer[buflen++] = *ptr;
                    ++ptr;
                }
                // stop, then set last char to null
                logf("End consuming at %p, *ptr = %c\n", ptr, *ptr);
                buffer[buflen] = '\0';

                // create new node
                logf("New subnode %s\n", buffer);
                csn_xpath_t *xnode = csn_xpath_new();
                csn_buf_write(xnode->tag, buffer);
                xnode->is_root = false;
                xnode->next = NULL;
                xnode->index = 0;

                // current node points to this node
                // set current node pointer to this node
                xptr->next = xnode;
                xptr = xnode;

                // reset buflen
                buflen = 0;
            }
        }
        if (*ptr == '[') {
            logs("Indexing, consuming\n");
            // consume until ]
            ++ptr;
            while (*ptr != ']' &&
                   *ptr != '\0') {
                if (!isdigit(*ptr)) {
                    goto _parse_error;
                }
                buffer[buflen++] = *ptr;
                ++ptr;
            }

            logf("End consuming at %p, *ptr = %c\n", ptr, *ptr);
            // stop, then set last char to null
            buffer[buflen] = '\0';

            // convert it to int;
            xptr->index = strtol(buffer, NULL, 10);
            logf("index: %d\n", xptr->index);
            //reset buflen
            buflen = 0;
            ++ptr;
        }
        if (*ptr == ']') {
            goto _parse_error;
        }
    } // while

    return root;

_parse_error:
    // free all allocated chunks
    csn_xpath_free(root);

    logs("Parsing error\n");
    return NULL;
}

csn_xpath_t *csn_xpath_new() {
    csn_xpath_t *ret = xalloc(sizeof(csn_xpath_t));
    ret->tag = csn_buf_new(1);

    return ret;
}

void csn_xpath_free(csn_xpath_t *xp) {
    csn_xpath_t *tmp, *xptr;
    xptr = xp;
    tmp = xptr;

    while (tmp) {
        csn_buf_free(tmp->tag);
        free(tmp);
        xptr = xptr->next;
        tmp = xptr;
    }
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

/* === buf_t implementation === */
buf_t *csn_buf_new(size_t size) {
    buf_t *ret = xalloc(sizeof(buf_t));

    ret->str = xcalloc(size + 1);
    ret->len = 0;

    return ret;
}

buf_t *csn_buf_from_str(const char *str) {
    int len = strlen(str);
    buf_t *buf = csn_buf_new(len + 1);

    memcpy(buf->str, str, len);
    buf->len = len;
    return buf;
}

char *csn_buf_write(buf_t *buf, const char *str) {
    int new_len = strlen(str);
    if (!buf) {
        logs("write to unallocated buf\n");
        buf = csn_buf_new(new_len + 1);
    }

    // realloc if new str differs
    if (new_len != buf->len) {
        buf->str = xrealloc(buf->str, new_len + 1);
    }
    buf->len = new_len;
    memcpy(buf->str, str, buf->len);
    buf->str[buf->len] = '\0';
    return buf->str;
}

char *csn_buf_write_char(buf_t *buf, const char c) {
    // write a char at 0 position without realloc
    buf->str[0] = c;
    buf->len = 1;
    buf->str[1] = '\0';

    return buf->str;
}

char *csn_buf_append(buf_t *buf, const char *str) {
    int old = buf->len;
    int new_len = strlen(str);
    buf->len += new_len; // new len
    buf->str = xrealloc(buf->str, buf->len + 1);

    memcpy(buf->str + old, str, new_len);
    buf->str[buf->len] = '\0';

    return buf->str;
}

char *csn_buf_append_char(buf_t *buf, const char c) {
    buf->str = xrealloc(buf->str, buf->len + 2);
    buf->str[buf->len] = c;
    buf->str[++(buf->len)] = '\0';

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
