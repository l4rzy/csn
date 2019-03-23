#include "internal.h"

static bool node_ok(TidyNode node) {
    TidyAttr attr;
    const char *node_name = tidyNodeGetName(node);
    if (node_name) {
        logs(node_name);
        for (attr = tidyAttrFirst(node); attr ; attr = tidyAttrNext(attr)) {
            const char *attr_name = tidyAttrName(attr);
            const char *attr_value = tidyAttrValue(attr);
            if (!strcmp(attr_name, "class") && !strcmp(attr_value, "tbtable")) {
                // first case to be matched, return immediately
                return true;
            }
        }
    }
    return false;
}

/* using DFS,
 * non-recursive algorithm
 * the technique here is to maintain two list, one for queued 1st gen childtren
 * and one for 2nd children
 */
static TidyNode traverse_to_node(TidyNode root) {
    // create 2 queues
    csn_queue_t *q1 = csn_queue_new();
    csn_queue_t *q2 = csn_queue_new();

    // add root to q1
    csn_enqueue(q1, root);

    // loop in q1 first, if !node_ok then add all its children to q2
    TidyNode curr, child;
    while ((curr = csn_dequeue(q1))) {
        if (node_ok(curr)) {
            return curr;
        }
        else {
            for (child = tidyGetChild(curr); child; child = tidyGetNext(child)) {
                csn_enqueue(q2, child);

            }
        }
        // swap q1 and q2, then start again
    }

    return NULL;
}


/* traverse to the xpath that's been parsed
 */
static TidyNode traverse_to_xpath(TidyDoc doc) {

}


csn_result_t *parse_search_result(TidyDoc doc, TidyNode tnod, int indent) {
    TidyNode node = traverse_to_node(tnod);
    if (!node) goto __parse_error;

    TidyNode child;
    TidyAttr attr;
    for (child = tidyGetChild(node); child; child = tidyGetNext(child)) {
        const char *node_name = tidyNodeGetName(child);
        logs(node_name);
        for (attr = tidyAttrFirst(child); attr; attr = tidyAttrNext(attr)) {
            printf("name: %s value %s\n", tidyAttrName(attr), tidyAttrValue(attr));
        }
    }
    return NULL;


__parse_error:
    puts("error!!!");
    return NULL;
}
