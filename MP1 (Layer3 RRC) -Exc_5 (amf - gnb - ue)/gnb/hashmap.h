﻿#ifndef HASHMAP_H
#define HASHMAP_H
#include<Windows.h>
#include "uthash.h"
#include"vector.h"
typedef struct {
    int key;           // key
    Vector value;      // vector giá trị
    UT_hash_handle hh; // giúp uthash quản lý phần tử trong bảng băm
} MapEntry;

extern MapEntry* map;

extern CRITICAL_SECTION map_cs;


void map_insert(int key, RRC value);
Vector* map_get(int key);
void map_print();

extern CRITICAL_SECTION map_cs;

#endif// hashmap.h