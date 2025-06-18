#ifndef GLOBAL_H
#define GLOBAL_H
#include<stdint.h>
#define SCALE 1
#define TCP_PORT 6000
#define numberOfNgapPerSecond 100000


typedef struct {
	uint32_t msg_type;// 100 là paging
	uint32_t ue_id;
	uint32_t tac;// 100 là tac đúng
	uint32_t cn_domain;// 100 cho gọi thoại, 101 cho data
}NgAP;

#endif