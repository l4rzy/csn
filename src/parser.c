#include "internal.h"

static csn_xpath_t *to_type, *to_name, *to_len, *to_count;

static void print_node(TidyDoc d, TidyNode n) {
    if (!n) {
        logs("Node doesn't exist\n");
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

/* parse in each entry in result of songs
 */
csn_result_t *parse_song_entry(TidyDoc doc, TidyNode tnod) {
    TidyAttr attr = tidyAttrFirst(tnod);
    if (!attr) {
        logs("This is the column header\n");
        goto _entry_error;
    }

    /* some variables
     */
    csn_result_t *ret = csn_result_new(true);
    TidyNode target;
#define TO_XPATH(x) \
    target = csn_xpath_traverse(tnod, x); \
    if (!target) { \
        logs("Could not traverse to xpath!\n"); \
        goto _entry_error; \
    }

    //type
    target = csn_xpath_traverse(tnod, to_type);
    if (!target) {
        ret->song->type = CSN_TYPE_SONG;
    }

    /* song name and artist
     * child of current node contains link and song name
     * next node contains artist
     */
    TO_XPATH(to_name);
#ifdef ENABLE_DEBUG
    print_node(doc, target);
#endif
    TidyNode tmp_node = tidyGetChild(target);
    if (!tmp_node) {
        goto _entry_error;
    }

    // link
    for (attr = tidyAttrFirst(tmp_node); attr; attr = tidyAttrNext(attr)) {
        if (!strcmp("href", tidyAttrName(attr))) {
            ret->song->link = csn_buf_from_str(tidyAttrValue(attr));
        }
    }

    // artist
    tmp_node = tidyGetChild(tmp_node);
    print_node(doc, tmp_node);

    return ret;

#undef TO_XPATH
_entry_error:
    return NULL;
}

csn_result_t *parse_song_search_result(TidyDoc doc) {
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
    TidyNode target = csn_xpath_traverse(root, search_xpath);
    if (!target) {
        logs("Could not traverse to xpath\n");
        goto _parse_error;
    }

    // prepare sub xpaths to traverse to
    to_type = csn_xpath_parse("/td[1]/p/a/img");
    to_name = csn_xpath_parse("/td[2]/div/div/p[1]");
    to_len = csn_xpath_parse("/td[3]/span");
    to_count = csn_xpath_parse("/td[6]/p[1]");

    // loop in all subnode, and call parse_entry() to generate a result node
    TidyNode child;
    for (child = tidyGetChild(target); child; child = tidyGetNext(child)) {
        csn_result_t *rslt = parse_song_entry(doc, child);
        if (rslt) {
            if (!rhead) {
                logs("Set result to head\n");
                // set that result as head
                rhead = rslt;
                rptr = rhead;
            }
            else {
                logs("Append new result\n");
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
