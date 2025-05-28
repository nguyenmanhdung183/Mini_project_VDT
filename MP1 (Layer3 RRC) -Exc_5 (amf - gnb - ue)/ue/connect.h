#ifndef CONNECT_H
#define CONNECT_H

#define _WINSOCK_DEPRECATED_NO_WARNINGS
#include"global.h"
#include<stdint.h>
#include<stdbool.h>
#include<winsock2.h>
#include <ws2tcpip.h>
#pragma comment(lib, "Ws2_32.lib")


SOCKET ue_socket;
struct sockaddr_in gnb_addr;





/// <fucion>
void udp_init();
bool receive_mib(SOCKET socket, MIB* mib, struct sockaddr_in* gnb_addr);
void increase_sfn(SystemParameter* sp);
uint16_t caculate_pf_mod_t(uint32_t ue_id);
void updateSFN(SystemParameter* s, MIB* mib);
bool receive_rrc(SOCKET socket, RRC* rrc, struct sockaddr_in* gnb_addr);



#endif