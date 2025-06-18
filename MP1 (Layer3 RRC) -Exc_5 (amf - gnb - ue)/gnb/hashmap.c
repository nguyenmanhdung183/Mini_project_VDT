#include"global.h"
#include"hashmap.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
MapEntry* map = NULL;  // map trống là bảng băm
CRITICAL_SECTION map_cs;

void map_insert(int key, RRC value) {
    EnterCriticalSection(&map_cs);

	MapEntry* e; // con trỏ đến MapEntry muốn thêm
    HASH_FIND_INT(map, &key, e);
    if (e == NULL) {
        e = malloc(sizeof(MapEntry));
        if (!e) {
            fprintf(stderr, "malloc error\n");
            LeaveCriticalSection(&map_cs);
            exit(1);
        }
        e->key = key;
        vector_init(&e->value); // Khởi tạo vector
		HASH_ADD_INT(map, key, e); // Thêm entry vào map
       // printf("New MapEntry created: key=%d, vector=%p\n", key, &e->value);
    }
    vector_push_back(&e->value, value); // Thêm vào vector
    //printf("Inserted value for key=%d, vector size=%zu\n", key, e->value.size);

    LeaveCriticalSection(&map_cs);
	//map_print(); // In ra map sau khi thêm phần tử mới
}

// Hàm xóa một entry khỏi map dựa trên key
void map_remove(int key) {
    EnterCriticalSection(&map_cs); // Bảo vệ truy cập đa luồng

    MapEntry* entry;
    HASH_FIND_INT(map, &key, entry); // Tìm entry với key tương ứng
    if (entry) {
        // Giải phóng bộ nhớ của vector
        if (entry->value.items) {
            free(entry->value.items); // Giải phóng mảng items
            entry->value.items = NULL;
        }
        entry->value.size = 0;
        entry->value.capacity = 0;
        // Xóa entry khỏi hash map
        HASH_DEL(map, entry);
        free(entry); // Giải phóng bộ nhớ của MapEntry
    }

    LeaveCriticalSection(&map_cs); // Giải phóng khóa
}

Vector* map_get(int key) { // Lấy giá trị tương ứng với khóa key từ map
    Vector* result = NULL;

    EnterCriticalSection(&map_cs);

    MapEntry* e;
    HASH_FIND_INT(map, &key, e);
    if (e != NULL) {
        result = &e->value;
    }
    LeaveCriticalSection(&map_cs);
    return result;
}


// Hàm in tất cả giá trị trong map
void map_print() {
    EnterCriticalSection(&map_cs); // Bảo vệ truy cập đa luồng

    MapEntry* entry, * tmp;
    HASH_ITER(hh, map, entry, tmp) { // Duyệt qua từng MapEntry trong map
        printf("Key: %d\n", entry->key);
        Vector* vec = &entry->value;
        printf("  Vector: size=%zu, capacity=%zu\n", vec->size, vec->capacity);

        // In từng phần tử RRC trong vector
        for (size_t i = 0; i < vec->size; i++) {
            RRC* item = &vec->items[i];
            printf("  Item %zu: msg_type=%u, cn_domain=%u, ue_id=%u, tac=%u\n",
                i, item->msg_type, item->cn_domain, item->ue_id, item->tac);
        }
    }
    LeaveCriticalSection(&map_cs); // Giải phóng khóa
}