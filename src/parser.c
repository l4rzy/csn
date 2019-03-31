#include "internal.h"

static void print_node(TidyNode n) {
    const char *name = tidyNodeGetName(n);
    printf("line: %d <%s ", tidyNodeLine(n), name);
    TidyAttr attr;
    for (attr = tidyAttrFirst(n); attr; attr = tidyAttrNext(attr)) {
        printf("%s=\"%s\" ", tidyAttrName(attr), tidyAttrValue(attr));
    }
    printf(">\n");
}

void trace_parent(TidyNode tnod) {
    TidyNode parent = tidyGetParent(tnod);

    if (!parent) {
        return;
    }

    print_node(parent);
    // recursive
    trace_parent(parent);
}

/* find the xpath of a node by tracing it
 */
void find_node(TidyNode tnod) {
    TidyNode child;
    TidyAttr attr;
    for (child = tidyGetChild(tnod); child; child = tidyGetNext(child) ) {
        ctmbstr name = tidyNodeGetName(child);
        if (name && !strcmp(name, "table")) {
            // get all its attributes
            for (attr = tidyAttrFirst(child); attr; attr = tidyAttrNext(attr)) {
                if (!strcmp(tidyAttrName(attr), "class") &&
                        !strcmp(tidyAttrValue(attr), "tbtable")) {
                    logs("Found it!\n");
                    print_node(child);
                    trace_parent(child);
                }
            }
        }
        find_node(child); /* recursive */
    }
}

void dump_node(TidyDoc doc, TidyNode tnod, int indent) {
    TidyNode child;
    TidyAttr attr;
    for (child = tidyGetChild(tnod); child; child = tidyGetNext(child) ) {
        switch (tidyNodeGetType(child)) {
        case TidyNode_Start:
            printf("%*s<%s ", indent, "", tidyNodeGetName(child));
            for (attr = tidyAttrFirst(child); attr; attr = tidyAttrNext(attr)) {
                printf("%s=\"%s\" ", tidyAttrName(attr), tidyAttrValue(attr));
            }
            printf(">\n");
            break;
        case TidyNode_End:
            printf("</%s>\n", tidyNodeGetName(child));
            break;
        case TidyNode_Text:
            do {
                TidyBuffer buf;
                tidyBufInit(&buf);
                tidyNodeGetText(doc, child, &buf);
                printf("%*s%s", indent, "", buf.bp);
                tidyBufFree(&buf);
            } while (0);
            break;
        default:
            break;

        }
        dump_node(doc, child, indent + 2); /* recursive */
    }
}

TidyNode traverse_to_xpath(TidyNode root, csn_xpath_t *xp) {
    int index;
    TidyNode node, child; // current node
    csn_xpath_t *xptr; // current pointer in xpath

    /* initialization
     */
    index = -1;
    node = root;
    xptr = xp->next; // skip the root node in xpath

_traverse_loop:
    logf("Current at xpath node: %s[%d]\n", xptr->tag->str, xptr->index);
    // stop condition
    if (xptr->next == NULL) {
        goto _traverse_exit;
    }

    // check all its children for the child that satisties the xpath
    //
    for (child = tidyGetChild(node); child; child = tidyGetNext(child)) {
        const char *child_name = tidyNodeGetName(child);
        if (!child_name) {
            continue;
        }

        logf("current child tag: %s\n", child_name);
        if ( !strcmp(xptr->tag->str, child_name)) {
            ++index;
            logf("Names matched, index is %d\n", index);
            if (xptr->index == index) {
                logs("Indice matched\n");
                print_node(child);
            } else {
                continue;
            }

            // starting again from the child
            // set current node to child, go to the next level of xpath
            node = child;
            index = -1;
            xptr = xptr->next;

            goto _traverse_loop;
        }
    }
    // there's no child that's satisfied
    logs("Found no satisfied child\n");
    goto _traverse_error;

_traverse_exit:
    logs("Traversed successfully!\n");
    return node;

_traverse_error:
    logs("Error traversing\n");
    return NULL;
}

/* parse in each entry in result of songs
 * scheme:
 *   1st child node: # of result
 *   2nd child node: song name and artist
 *   3rd: duration and max quality
 *   6th: download count
 * let's dig it
 */
csn_result_t *parse_song_entry(TidyDoc doc, TidyNode tnod) {
    TidyNode child;
    int i;

    TidyAttr attr = tidyAttrFirst(tnod);
    if (!attr) {
        logs("This is the column header\n");
        return NULL;
    }

    csn_xpath_t *to_name = csn_xpath_parse("/div/div/p/a/end");
    // create new result as a song
    csn_result_t *r = csn_result_new(true);

    for (child = tidyGetChild(tnod), i = 0;
            child;
            child = tidyGetNext(child), ++i) {

        switch (i) {
        case 1: // song name & artist
            dump_node(doc, child, 0);
            TidyNode tg = traverse_to_xpath(child, to_name);
            print_node(tg);
            break;

        case 2: // duration and max quality
            do {
                TidyNode c = tidyGetChild(child);
                dump_node(doc, c, 0);
            } while (0);
            puts("======");
            break;

        case 5: // download count
            do {
                TidyNode c = tidyGetChild(child);
                dump_node(doc, c, 0);
            } while (0);
            puts("===");
            break;
        default:
            break;
        }
    }

    // free xpath
    csn_xpath_free(to_name);
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
    csn_xpath_free(search_xpath);
#ifdef ENABLE_DEBUG
    _t_end = clock();

    logf("Took %ld to finish\n", _t_end - _t_start);
#endif

    return rhead;
_parse_error:
    return NULL;
}
