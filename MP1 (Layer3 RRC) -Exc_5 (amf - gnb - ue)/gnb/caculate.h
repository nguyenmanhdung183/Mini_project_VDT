#ifndef CACULATE_H
#define CACULATE_H
#include<Windows.h>
#include "stdint.h"
#include"global.h"


extern SystemClock clk;
extern volatile long rrc_sent;

extern SOCKET tcp_sock, udp_sock, udp_sock_rrc, amf_sock;
extern struct sockaddr_in gnb_addr_tcp, gnb_addr_udp, gnb_addr_udp_rrc, ue_addr, ue_addr_rrc, amf_addr;


void increase_sfn(SystemParameter* sp);
uint16_t caculate_pf_mod_t(uint32_t ue_id);
//PagingTask calc_task(NgAP ngap); //return lại task khi đẩy ngap vào
RrcParmeter calculate_rrc_parameter(NgAP ngap, uint16_t sfn, uint8_t sf);// tính toán PF, PO để lên lịch
void broadcast_rrc(int ue_id);


unsigned __stdcall clock_thread(void* arg);
unsigned __stdcall worker(void* arg); 
unsigned __stdcall log_thread(void* arg);
unsigned __stdcall scheduler(void* arg);
unsigned __stdcall listening_amf(void* arg);
unsigned __stdcall broadcast_mib_thread(void* arg);
#endif