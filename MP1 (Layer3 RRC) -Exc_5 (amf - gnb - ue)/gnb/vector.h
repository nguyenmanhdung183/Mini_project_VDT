#ifndef VECTOR_H
#define VECTOR_H
#include"global.h"

typedef struct {
    RRC* items;
    size_t size;
    size_t capacity;
} Vector;

void vector_init(Vector* v);
void vector_push_back(Vector* v, RRC item);
void vector_free(Vector* v);

/*
    var v1 = {10};
    var v2 = {20};

    map_insert(1, v1);
    map_insert(1, v2);

    Vector* vec = map_get(1);
    if (vec) {
        for (size_t i = 0; i < vec->size; i++) {
            printf("Value at index %zu = %d\n", i, vec->items[i].data);
        }
    }
}


*/
#endif// VECTOR_H