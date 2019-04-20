#include "internal.h"
#include <ctype.h>

void *_xalloc(size_t size) {
    void *ret = malloc(size);
    if (!ret) {
        fatals("Could not allocate memory\n");
    }
    return ret;
}

void *_xcalloc(size_t size) {
    void *ret = calloc(size, 1);

    if (!ret) {
        fatals("Could not allocate memory\n");
    }
    return ret;
}

void *_xrealloc(void *ptr, size_t new_size) {
    ptr = realloc(ptr, new_size);
    if (!ptr) {
        fatals("Could not allocate memory\n");
    }
    return ptr;
}
