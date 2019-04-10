#include "internal.h"

static csn_xpath_t *to_type, *to_name, *to_len, *to_count;

/* get text of a node
 */
static char *get_text(TidyDoc doc, TidyNode nod) {
    // move to the "child" node
    TidyNode child = tidyGetChild(nod);
    if (!child) {
        return NULL;
    }

    // text node
    TidyBuffer buf;
    tidyBufInit(&buf);
    tidyNodeGetText(doc, child, &buf);
    char *ret = strdup((char*)buf.bp);
    tidyBufFree(&buf);

    return ret;
}

/* get attribute value of a node
 */
static char *get_attr(TidyNode nod, const char *sattr) {
    TidyAttr attr;
    if (!nod) {
        return NULL;
    }
    for (attr = tidyAttrFirst(nod); attr; attr = tidyAttrNext(attr)) {
        if (!strcmp(sattr, tidyAttrName(attr))) {
            return strdup(tidyAttrValue(attr)); // not sure about mem leak here
        }
    }
    return NULL;
}

/* get first child with name
 */
static TidyNode get_child(TidyNode nod, const char *sname) {
    if (!nod) {
        return NULL;
    }
    TidyNode child;

    for (child = tidyGetChild(nod); child; child = tidyGetNext(child)) {
        const char *name = tidyNodeGetName(child);
        if (name && !strcmp(name, sname)) {
            return child;
        }
    }
    return NULL;
}

static void print_node(TidyDoc doc, TidyNode nod) {
    if (!nod) {
        logs("Node doesn't exist\n");
        return;
    }
    const char *name = tidyNodeGetName(nod);
    if (name) {
        printf("line: %d <%s ", tidyNodeLine(nod), name);
        TidyAttr attr;
        for (attr = tidyAttrFirst(nod); attr; attr = tidyAttrNext(attr)) {
            printf("%s=\"%s\" ", tidyAttrName(attr), tidyAttrValue(attr));
        }
        printf(">\n");
    }
}

/* parse in each entry in result of songs
 * WARN: shitty code, need to clean up
 */
csn_result_t *parse_song_entry(TidyDoc doc, TidyNode tnod) {
#ifdef ENABLE_DEBUG
    _t_start = clock();
#endif
    TidyAttr attr = tidyAttrFirst(tnod);
    if (!attr) {
        logs("This is the column header\n");
        goto _entry_error;
    }

    /* some variables
     */
    csn_result_t *ret = csn_result_new(true);
    TidyNode target;
    TidyNode tmp_node;
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
    tmp_node = tidyGetChild(target);
    if (!tmp_node) {
        goto _entry_error;
    }

    // link
    char *link = get_attr(tmp_node, "href");
    if (link) {
        ret->song->link = csn_buf_possess(link);
    }

    // name
    char *name = get_text(doc, tmp_node);
    ret->song->title = csn_buf_possess(name);
    csn_buf_trim(ret->song->title);

    // artist
    tmp_node = tidyGetNext(target);
    char *artist = get_text(doc, tmp_node);
    ret->song->artist = csn_buf_possess(artist);
    csn_buf_trim(ret->song->artist);

    // length
    TO_XPATH(to_len);
    char *len = get_text(doc, target);
    ret->song->duration = csn_buf_possess(len);
    csn_buf_trim(ret->song->duration);

    // max quality
    TidyNode mq_node = get_child(target, "span");
    if (mq_node){
        char *max_quality = get_text(doc, mq_node);
        ret->song->max_quality = csn_buf_possess(max_quality);
        csn_buf_trim(ret->song->max_quality);
    }

    // download count
    TO_XPATH(to_count);
    char *count = get_text(doc, target);
    ret->song->download_count = csn_buf_possess(count);
    csn_buf_trim(ret->song->download_count);

#ifdef ENABLE_DEBUG
    _t_end = clock();
    logf("Took %ld to complete\n", _t_end - _t_start);
#endif
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
