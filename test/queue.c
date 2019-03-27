/* for testing queue
 */
#include <csn.h>
#include "../src/internal.h"

int queue_count(csn_queue_t *q) {
    csn_node_t *node = q->head;
    int count = 0;
    while (1) {
        if (!node) {
            break;
        }
        else {
            ++count;
            node = node->next;
        }
    }
    return count;
}


int main() {
    TidyNode n = NULL;
    csn_queue_t *queue = csn_queue_new();

    logs("Inserting 100 nodes to queue\n");
    for (int i = 0; i < 100; ++i) {
        csn_enqueue(queue, n);
    }
    ASSERT(queue_count(queue) == 100);

    logs("Removing 50 nodes from queue\n");
    for (int i = 0; i < 50; ++i) {
        csn_dequeue(queue);
    }
    ASSERT(queue_count(queue) == 50);

    logs("Removing 50 nodes from queue\n");
    for (int i = 0; i < 50; ++i) {
        csn_dequeue(queue);
    }
    ASSERT(queue_count(queue) == 0);

    csn_queue_free(queue);
    return 0;
}
