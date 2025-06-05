#ifndef GLOBAL_H
#define GLOBAL_H

#include<stdint.h>

#define SCALE 10 // scale thời gian lên để dễ log terminal
#define DRX 128 // tại sfn 0 128 256,512 thức để nhận paging
#define UE_ID 123
#define Ns 1 //số PO trong PF

#define UDP_PORT 5500

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

typedef enum {
    SYNC,
    ASYNC
}State;

#endif