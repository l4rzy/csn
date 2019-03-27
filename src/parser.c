#include "internal.h"

#ifdef ENABLE_DEBUG
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
#endif

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
    // there's no child that's sufficicent
    logs("Found no sufficicent child\n");
    goto _traverse_error;

_traverse_exit:
    logs("Traversed successfully!\n");
    return node;

_traverse_error:
    logs("Error traversing\n");
    return NULL;
}


csn_result_t *parse_search_result(TidyDoc doc) {
    // get root node from document, and pass it to the traversing function
    // we'll get
    TidyNode root = tidyGetRoot(doc);

    puts("===========================================");
    find_node(root);
    puts("===========================================");

    logs("Parsing xpath\n");
    csn_xpath_t *search_xpath = csn_xpath_parse(CSN_SEARCH_XPATH);
    logs("Traversing to xpath\n");
    TidyNode result = traverse_to_xpath(root, search_xpath);

    if (result) {

    }

    csn_xpath_free(search_xpath);

    return NULL;
}
