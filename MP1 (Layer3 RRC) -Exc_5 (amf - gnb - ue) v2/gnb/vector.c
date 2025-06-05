#include"vector.h"
#include"stdio.h"
#include <stdlib.h>

void vector_init(Vector* v) {
    v->items = NULL;
    v->size = 0;
    v->capacity = 0;
   // printf("Vector initialized: items=%p, size=%zu, capacity=%zu\n", v->items, v->size, v->capacity);
}

void vector_push_back(Vector* v, RRC item) {
    if (v == NULL) {
        fprintf(stderr, "Lỗi: Con trỏ Vector là NULL\n");
        exit(1);
    }
    if (v->items == NULL && v->capacity > 0) {
        fprintf(stderr, "Lỗi: v->items là NULL nhưng capacity != 0\n");
        exit(1);
    }
    //printf("sizeof(RRC) = %zu\n", sizeof(RRC));
    if (v->size == v->capacity) {
        size_t new_capacity = v->capacity == 0 ? 4 : v->capacity * 2;
        //printf("Allocating: new_capacity=%zu, size=%zu bytes\n", new_capacity, new_capacity * sizeof(RRC));
        RRC* new_items = malloc(new_capacity * sizeof(RRC));
        if (!new_items) {
            fprintf(stderr, "Lỗi: Cấp phát bộ nhớ thất bại\n");
            exit(1);
        }
       // printf("malloc returned: %p\n", new_items);
        // Kiểm tra tính hợp lệ của bộ nhớ
        *(char*)new_items = 0; // Thử ghi byte đầu tiên
       // printf("Test write to new_items successful\n");
        memset(new_items, 0, new_capacity * sizeof(RRC)); // Xóa bộ nhớ
        if (v->items) {
            memcpy(new_items, v->items, v->size * sizeof(RRC)); // Sao chép dữ liệu cũ
            free(v->items); // Giải phóng bộ nhớ cũ
        }
        v->items = new_items;
        v->capacity = new_capacity;
    }
   // printf("Before push: items=%p, size=%zu, capacity=%zu\n", v->items, v->size, v->capacity);
   // printf("Pushing item: msg_type=%u, cn_domain=%u, ue_id=%u, tac=%u\n", item.msg_type, item.cn_domain, item.ue_id, item.tac);
    v->items[v->size++] = item;
   // printf("push_back done: size=%zu, capacity=%zu\n", v->size, v->capacity);
}
void vector_free(Vector* v) {
    free(v->items);
    v->items = NULL;
    v->size = 0;
    v->capacity = 0;
}
