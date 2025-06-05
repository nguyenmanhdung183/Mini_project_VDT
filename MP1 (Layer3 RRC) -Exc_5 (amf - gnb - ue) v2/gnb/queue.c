#include"queue.h"

CRITICAL_SECTION queue_lock, sfn_lock;
int front = 0, rear = 0;


bool is_empty() { return front == rear; }
bool is_full() { return (rear + 1) % MAX_QUEUE == front; }

void enqueue(NgAP ngap) {
    EnterCriticalSection(&queue_lock);
    if (!is_full()) {
        queue[rear] = ngap;
        rear = (rear + 1) % MAX_QUEUE;
    }
    LeaveCriticalSection(&queue_lock);
}


bool dequeue(NgAP* ngap) {
    EnterCriticalSection(&queue_lock);
    if (is_empty()) {
        LeaveCriticalSection(&queue_lock);
        return false;
    }
    *ngap = queue[front];
    front = (front + 1) % MAX_QUEUE;
    LeaveCriticalSection(&queue_lock);
    return true;
}
