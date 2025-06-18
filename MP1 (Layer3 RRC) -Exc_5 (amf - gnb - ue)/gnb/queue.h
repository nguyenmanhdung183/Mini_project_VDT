#ifndef QUEUE_H
#define QUEUE_H

#include"global.h"
#include<stdbool.h>
#include <process.h>
#include <Windows.h>



#define MAX_QUEUE 16384 // 2^14
NgAP queue[MAX_QUEUE];
extern int rear;
extern int front;
extern CRITICAL_SECTION queue_lock;
extern CRITICAL_SECTION sfn_lock;

bool is_empty();
bool is_full();
void enqueue(NgAP ngap);
bool dequeue(NgAP* ngap);

#endif
