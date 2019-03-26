#include "internal.h"

#ifdef ENABLE_DEBUG
static void print_node(TidyNode n) {
    const char *name = tidyNodeGetName(n);
    logf("node name: %s at line %d\n", name, tidyNodeLine(n));
    TidyAttr attr;
    for (attr = tidyAttrFirst(n); attr; attr = tidyAttrNext(attr)) {
        printf("%s: %s\n", tidyAttrName(attr), tidyAttrValue(attr));
    }
}

/* find the xpath of a node by tracing it
 */
void trace_xpath(TidyDoc doc, TidyNode tnod, int indent)
{
    TidyNode child;
    for (child = tidyGetChild(tnod); child; child = tidyGetNext(child) ) {
        ctmbstr name = tidyNodeGetName(child);
        if (name) {
            /* if it has a name, then it's an HTML tag ... */
            TidyAttr attr;
            printf("%*.*s%s ", indent, indent, "<", name);
            /* walk the attribute list */
            for (attr = tidyAttrFirst(child); attr; attr = tidyAttrNext(attr) ) {
                printf(tidyAttrName(attr));
                tidyAttrValue(attr) ? printf("=\"%s\" ",
                                             tidyAttrValue(attr)) : printf(" ");
            }
            printf(">\n");
        }
        else {
            /* if it doesn't have a name, then it's probably text, cdata, etc... */
            TidyBuffer buf;
            tidyBufInit(&buf);
            tidyNodeGetText(doc, child, &buf);
            printf("%*.*s\n", indent, indent, buf.bp ? (char *)buf.bp : "");
            tidyBufFree(&buf);
        }
        trace_xpath(doc, child, indent + 4); /* recursive */
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
    trace_xpath(doc, root, 0);
    puts("===========================================");

    logs("Parsing xpath\n");
    csn_xpath_t *search_xpath = csn_xpath_parse(CSN_SEARCH_XPATH);
    logs("Traversing to xpath\n");
    TidyNode result = traverse_to_xpath(root, search_xpath);

#ifdef ENABLE_DEBUG
    print_node(result);
#endif

    csn_xpath_free(search_xpath);

    return NULL;
}
