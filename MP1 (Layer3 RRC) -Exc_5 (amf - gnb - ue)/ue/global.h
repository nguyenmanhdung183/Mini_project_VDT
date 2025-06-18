#ifndef GLOBAL_H
#define GLOBAL_H

#include<stdint.h>

#define SCALE 1 // scale thời gian lên để dễ log terminal
#define DRX 128 // tại sfn 0 128 256,512 thức để nhận paging
#define UE_ID 300
#define Ns 2 //số PO trong PF

#define N0 (DRX / Ns)
#define PF ((DRX / N0) * (UE_ID % N0)) // 123

#define is  (UE_ID / N0) % Ns
#define PO  table[Ns][is] //9 

#define UDP_PORT 5500
#define UDP_PORT_RRC 5501

#define INTERVAL_MS 1 // mô phỏng slot 1ms

#define my_id 123
#define my_tac 100


typedef struct {
    uint32_t msg_type; // 100 là paging
    uint32_t ue_id;
    uint32_t tac;// 100 là tac đúng
    uint32_t cn_domain;// 100 cho gọi thoại, 101 cho data
}RRC;

typedef struct {
    uint8_t message_id;
    uint16_t sfn_value;
} MIB;
#pragma pack(pop)

typedef struct {
    unsigned int  time;
    uint16_t sfn;
} SystemParameter;

typedef struct {
    uint64_t time_ms;
    uint16_t sfn;
    uint8_t sf; // mô phỏng 1ms slot trong mỗi SFN
} SystemClock;

typedef enum {
    SYNC,
    ASYNC
}State;




#endif