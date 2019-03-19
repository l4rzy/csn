#include "internal.h"

csn_result_t *parse_search_result(TidyDoc doc, TidyNode tnod, int indent)
{
    TidyNode child;
    for (child = tidyGetChild(tnod); child; child = tidyGetNext(child) ) {
        ctmbstr name = tidyNodeGetName(child);
        if (name && !strcmp(name, "tr")) {
            TidyAttr attr;
            /* walk the attribute list */
            for (attr = tidyAttrFirst(child); attr; attr = tidyAttrNext(attr) ) {
                printf("%s: %s\n", tidyAttrName(attr), tidyAttrValue(attr));
            }
        }
        else {
            /* if it doesn't have a name, then it's probably text, cdata, etc... */
            TidyBuffer buf;
            tidyBufInit(&buf);
            tidyNodeGetText(doc, child, &buf);
            //printf("%*.*s\n", indent, indent, buf.bp?(char *)buf.bp:"");
            tidyBufFree(&buf);
        }
        dumpNode(doc, child, indent + 4); /* recursive */
    }
}
