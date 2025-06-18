#ifndef GLOBAL_H
#define GLOBAL_H
#include<stdint.h>
#define SCALE 1
#define UDP_PORT 5500
#define UDP_PORT_RRC 5501
#define TCP_PORT 6000
#define UE_ID 300
#define DRX 128 //1280s
#define Ns 2 //số PO trong PF

#define INTERVAL_MS 1 // mô phỏng slot 1ms
#define WORKER_COUNT 4


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

typedef struct {
    uint64_t time_ms;
    uint16_t sfn;
    uint8_t sf; // mô phỏng 1ms slot trong mỗi SFN
} SystemClock;


typedef struct {
    uint16_t PF;
    uint8_t is;
	uint8_t PO; // subframe
}RrcParmeter;

///<var>

extern SystemClock clk;
extern int8_t table[5][4];
extern volatile long rrc_sent;
///</var>

#endif