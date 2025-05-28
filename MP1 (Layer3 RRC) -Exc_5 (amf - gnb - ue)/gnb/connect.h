#ifndef CONNECT_H
#define CONNECT_H

#define _WINSOCK_DEPRECATED_NO_WARNINGS
#include"global.h"
#include<stdint.h>
#include<stdbool.h>
#include<winsock2.h>
#include <ws2tcpip.h>
#pragma comment(lib, "Ws2_32.lib")

SOCKET tcp_sock, udp_sock, amf_sock;
struct sockaddr_in gnb_addr_tcp, gnb_addr_udp, ue_addr, amf_addr;


void increase_sfn(SystemParameter* sp);
uint16_t caculate_pf_mod_t(uint32_t ue_id);
void udp_tcp_init();
bool receive_ngap(SOCKET socket, NgAP* ngap);
bool send_rrc(SOCKET socket, const RRC* rrc, struct sockaddr_in* ue_addr);
bool send_mib(SOCKET socket, const MIB* mib, struct sockaddr_in* ue_addr);

#endif