#ifndef CONNECT_H
#define CONNECT_H

#include<winsock2.h>
#include<time.h>
#include <ws2tcpip.h>
#include"global.h"
#include<stdbool.h>
#pragma comment(lib, "Ws2_32.lib")

WSADATA wsaData;
SOCKET tcp_sock;
struct sockaddr_in gnb_addr;


void tcp_init();
bool send_ngap(SOCKET sock, const  NgAP* ngap);

#endif