#include "internal.h"
#include <ctype.h>

/* parse xpath into linked list, this support a subset of xpath
 * and for use locally, so I guess this implementation is ok :)
 */
csn_xpath_t *csn_xpath_parse(const char *str) {
    logf("Parsing `%s`\n", str);
#ifdef ENABLE_DEBUG
    _t_start = clock();
#endif
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
                while (*ptr != '/' &&
                        *ptr != '[' &&
                        *ptr != ']' &&
                        *ptr != '\0') {
                    buffer[buflen++] = *ptr;
                    ++ptr;
                }
                // stop, then set last char to null
                buffer[buflen] = '\0';

                // create new node
                csn_xpath_t *xnode = csn_xpath_new();
                csn_buf_write(xnode->tag, buffer);
                xnode->is_root = false;
                xnode->next = NULL;
                xnode->index = 1; // predicate set to one

                // current node points to this node
                // set current node pointer to this node
                xptr->next = xnode;
                xptr = xnode;

                // reset buflen
                buflen = 0;
            }
        }
        if (*ptr == '[') {
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

            // stop, then set last char to null
            buffer[buflen] = '\0';

            // convert it to int;
            xptr->index = strtol(buffer, NULL, 10);
            //reset buflen
            buflen = 0;
            ++ptr;
        }
        if (*ptr == ']') {
            goto _parse_error;
        }
    } // while

#ifdef ENABLE_DEBUG
    _t_end = clock();
    logf("Took %ld to complete\n", _t_end - _t_start);
#endif
    return root;

_parse_error:
    // free all allocated chunks
    csn_xpath_free(root);

    logs("Parsing error\n");
    return NULL;
}

/* traverses from a node to local xpath
 */
TidyNode csn_xpath_traverse(TidyNode root, csn_xpath_t *xp) {
#ifdef ENABLE_DEBUG
    _t_start = clock();
#endif
    int index;
    TidyNode node, child; // current node
    csn_xpath_t *xptr; // current pointer in xpath

    /* initialization
     */
    index = 0;
    node = root;
    xptr = xp->next; // skip the root node in xpath

_traverse_loop:
    // stop condition
    if (xptr == NULL) {
        goto _traverse_exit;
    }
    logf("Current at xpath node: %s[%d]\n", xptr->tag->str, xptr->index);

    // check all its children for the child that satisties the xpath
    //
    for (child = tidyGetChild(node); child; child = tidyGetNext(child)) {
        const char *child_name = tidyNodeGetName(child);
        if (!child_name) {
            continue;
        }

        if (!strcmp(xptr->tag->str, child_name)) {
            ++index;
            // next index in case they didn't match
            if (xptr->index != index) {
                continue;
            }
            logf("Current child tag: %s[%d] => matching\n", child_name, index);
            // starting again from the child
            // set current node to child, go to the next level of xpath
            node = child;
            index = 0;
            xptr = xptr->next;

            goto _traverse_loop;
        }
    }
    // there's no child that's satisfied
    logs("Found no satisfied child\n");
    goto _traverse_error;

_traverse_exit:
#ifdef ENABLE_DEBUG
    _t_end = clock();

    logf("Took %ld to finish\n", _t_end - _t_start);
#endif
    logs("Traversed successfully!\n");
    return node;

_traverse_error:
    logs("Error traversing\n");
    return NULL;
}

csn_xpath_t *csn_xpath_new() {
    csn_xpath_t *ret = xalloc(sizeof(csn_xpath_t));
    ret->tag = csn_buf_new(1);

    return ret;
}

/* recursive structure, free the head itself
 */
void csn_xpath_free(csn_xpath_t *xp) {
    logf("Freeing xpath at %p\n", xp);
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
