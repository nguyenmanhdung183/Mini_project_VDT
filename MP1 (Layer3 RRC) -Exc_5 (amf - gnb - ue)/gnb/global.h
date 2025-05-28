#ifndef GLOBAL_H
#define GLOBAL_H
#include<stdint.h>

#define SCALE 10
#define UDP_PORT 5500
#define TCP_PORT 6000
#define UE_ID 123
#define DRX 128 //1280s
#define Ns 1 //số PO trong PF

typedef struct {
    uint32_t msg_type;// 100 là paging
    uint32_t ue_id;
    uint32_t tac;// 100 là tac đúng
    uint32_t cn_domain;// 100 cho gọi thoại, 101 cho data
}NgAP;

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

///<var>

///</var>


#endif