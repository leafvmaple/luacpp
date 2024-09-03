#include "lzio.h"
#include <malloc.h>

int char2int(char c) {
    return static_cast<int>(c);
}

int zgetc(ZIO* z) {
    if (z->n-- > 0)
        return char2int(*z->p++);
    return luaZ_fill(z);
}

int luaZ_fill(ZIO* z) {
    size_t size = 0;
    const char* buff = z->reader(z->L, z->data, &size);
    if (!buff || !size)
        return EOZ;
    z->n = size - 1;
    z->p = buff;
    return (*(z->p++));
}

