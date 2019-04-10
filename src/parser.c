#include "internal.h"

static csn_xpath_t *to_name, *to_len, *to_count;

static void print_node(TidyDoc d, TidyNode n) {
    if (!n) {
        return;
    }
    const char *name = tidyNodeGetName(n);
    if (name) {
        printf("line: %d <%s ", tidyNodeLine(n), name);
        TidyAttr attr;
        for (attr = tidyAttrFirst(n); attr; attr = tidyAttrNext(attr)) {
            printf("%s=\"%s\" ", tidyAttrName(attr), tidyAttrValue(attr));
        }
        printf(">\n");
    }
    else {
        // text node
        TidyBuffer buf;
        tidyBufInit(&buf);
        tidyNodeGetText(d, n, &buf);
        printf("%s\n", buf.bp);
        tidyBufFree(&buf);
    }
}

/* traverses from a node to local xpath
 */
TidyNode traverse_to_xpath(TidyNode root, csn_xpath_t *xp) {
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

        logf("Current child tag: %s[%d]\n", child_name, index);
        if (!strcmp(xptr->tag->str, child_name)) {
            ++index;
            // next index in case they didn't match
            if (xptr->index != index) {
                continue;
            }
            logs("Matching\n");
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

/* parse in each entry in result of songs
 */
csn_result_t *parse_song_entry(TidyDoc doc, TidyNode tnod) {
    TidyNode child;
    int i;

    TidyAttr attr = tidyAttrFirst(tnod);
    if (!attr) {
        logs("This is the column header\n");
        goto _entry_error;
    }

    TidyNode target;
#define TO_XPATH(x) \
    target = traverse_to_xpath(tnod, x); \
    if (!target) { \
        logs("Could not traverse to xpath!\n"); \
        goto _entry_error; \
    }
    /* song name and artist
     * child of current node contains link and song name
     * next node contains artist
     */
    TO_XPATH(to_name);
#ifdef ENABLE_DEBUG
    print_node(doc, target);
#endif


#undef TO_XPATH
_entry_error:
    return NULL;
}

csn_result_t *parse_search_result(TidyDoc doc) {
#ifdef ENABLE_DEBUG
    _t_start = clock();
#endif
    // get root node from document, and pass it to the traversing function
    // we'll get
    TidyNode root = tidyGetRoot(doc);
    csn_result_t *rhead = NULL;
    csn_result_t *rptr = NULL;

    logs("Parsing xpath\n");
    csn_xpath_t *search_xpath = csn_xpath_parse(CSN_SEARCH_XPATH);
    logs("Traversing to xpath\n");

    // the target node we wanted to traverse to
    TidyNode target = traverse_to_xpath(root, search_xpath);
    if (!target) {
        logs("Could not traverse to xpath\n");
        goto _parse_error;
    }

    // prepare sub xpaths to traverse to
    to_name = csn_xpath_parse("/td[2]/div/div/p[1]");
    to_len = csn_xpath_parse("/td[3]/span");
    to_count = csn_xpath_parse("/td[6]/p[1]");

    // loop in all subnode, and call parse_entry() to generate a result node
    TidyNode child;
    for (child = tidyGetChild(target); child; child = tidyGetNext(child)) {
        csn_result_t *rslt = parse_song_entry(doc, child);
        if (rslt) {
            if (!rhead) {
                // set that result as head
                rhead = rslt;
                rptr = rhead;
            }
            else {
                // link to rptr
                rptr->next = rslt;
                rptr = rslt;
            }
        }
        else {
            logs("This child doesn't return any results\n");
        }
    }

    // free xpaths
    csn_xpath_free(search_xpath);
    csn_xpath_free(to_name);
    csn_xpath_free(to_len);
    csn_xpath_free(to_count);

#ifdef ENABLE_DEBUG
    _t_end = clock();

    logf("Took %ld to finish\n", _t_end - _t_start);
#endif

    return rhead;
_parse_error:
    return NULL;
}
