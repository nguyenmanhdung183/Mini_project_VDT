#ifndef CONNECT_H
#define CONNECT_H

#define _WINSOCK_DEPRECATED_NO_WARNINGS
#include"global.h"
#include<stdint.h>
#include<stdbool.h>
#include<winsock2.h>
#include <ws2tcpip.h>
#pragma comment(lib, "Ws2_32.lib")


SOCKET ue_socket, ue_socket_rrc;
struct sockaddr_in gnb_addr, gnb_addr_rrc;





/// <fucion>
bool check_correct_rrc_time(uint16_t sfn, uint8_t sf);
void udp_init();// udp cho mib
void udp_init_rrc(); // udp cho rrc
bool receive_mib(SOCKET socket, MIB* mib, struct sockaddr_in* gnb_addr);
void increase_sfn(SystemParameter* sp);
uint16_t caculate_pf_mod_t(uint32_t ue_id);
void updateSFN(SystemParameter* s, MIB* mib);
bool receive_rrc(SOCKET socket, RRC* rrc, struct sockaddr_in* gnb_addr);


unsigned __stdcall receive_mib_thread(void* arg);
unsigned __stdcall receive_rrc_thread(void* arg);
unsigned __stdcall clock_thread(void* arg);


// luông chính để nhận MIB, luồng phụ để nhận RRC

#endif