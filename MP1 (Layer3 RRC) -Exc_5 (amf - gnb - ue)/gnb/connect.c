﻿#include"connect.h"



void udp_tcp_init() {

    // khởi tạo wsadata tcp/udp
    WSADATA wsaData;

    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        printf("WSAStartup failed\n");
        return;
    }


    // tạo socket cho tcp/udp
    udp_sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
	udp_sock_rrc = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    tcp_sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

    if (udp_sock == INVALID_SOCKET || tcp_sock == INVALID_SOCKET || udp_sock_rrc == INVALID_SOCKET) {
        printf("Cannot create socket\n");
		closesocket(udp_sock);
		closesocket(tcp_sock);
		closesocket(udp_sock_rrc);
        WSACleanup();
        return;
    }

    //gán địa chỉ cho tcp  và udp
    gnb_addr_udp.sin_family = AF_INET;
    gnb_addr_udp.sin_port = htons(UDP_PORT);  // 5500
    gnb_addr_udp.sin_addr.s_addr = INADDR_ANY;


    gnb_addr_udp_rrc.sin_family = AF_INET;
    gnb_addr_udp_rrc.sin_port = htons(UDP_PORT_RRC);  // 
    gnb_addr_udp_rrc.sin_addr.s_addr = INADDR_ANY;


    gnb_addr_tcp.sin_family = AF_INET;
    gnb_addr_tcp.sin_port = htons(TCP_PORT);  // 6000
    gnb_addr_tcp.sin_addr.s_addr = INADDR_ANY;



    // Bind TCP
    if (bind(tcp_sock, (struct sockaddr*)&gnb_addr_tcp, sizeof(gnb_addr_tcp)) == SOCKET_ERROR) {
        printf("Bind TCP failed: %d\n", WSAGetLastError());
        closesocket(udp_sock);
        WSACleanup();
        return;
    }
    printf("TCP gNB start...\n");

    // Bind UDP 
    if (bind(udp_sock, (struct sockaddr*)&gnb_addr_udp, sizeof(gnb_addr_udp)) == SOCKET_ERROR) {
        printf("Bind UDP failed: %d\n", WSAGetLastError());
        closesocket(tcp_sock);
        closesocket(udp_sock);
        WSACleanup();
        return;
    }
    printf("UDP gNB start...\n");

	// Bind UDP RRC
    if (bind(udp_sock_rrc, (struct sockaddr*)&gnb_addr_udp_rrc, sizeof(gnb_addr_udp_rrc)) == SOCKET_ERROR) {
        printf("Bind UDP RRC failed: %d\n", WSAGetLastError());
        closesocket(tcp_sock);
        closesocket(udp_sock);
		closesocket(udp_sock_rrc);
        WSACleanup();
		return;
    }

    // Listen TCP connection
    if (listen(tcp_sock, SOMAXCONN) == SOCKET_ERROR) {
        printf("listen AMF failed\n");
        closesocket(tcp_sock);
        closesocket(udp_sock);
        WSACleanup();
        return;
    }

    // Accept  AMF connection
    int amf_addr_size = sizeof(amf_addr);
    amf_sock = accept(tcp_sock, (struct sockaddr*)&amf_addr, &amf_addr_size);
    if (amf_sock == INVALID_SOCKET) {
        printf("accept AMF failed\n");
        closesocket(tcp_sock);
        closesocket(udp_sock);
        WSACleanup();
        return;
    }
    printf("amf connected.\n");

    //  UDP receive
    char buff[1024];
    int ue_addr_len = sizeof(ue_addr);
    memset(buff, 0, sizeof(buff));
    int r = recvfrom(udp_sock, buff, sizeof(buff), 0, (struct sockaddr*)&ue_addr, &ue_addr_len);
    if (r == SOCKET_ERROR) {
        printf("recv failed\n");
        return;
    }
    buff[r] = '\0';
    printf("Received from ue [%s:%d]: %s\n",
        inet_ntoa(ue_addr.sin_addr), ntohs(ue_addr.sin_port), buff);




	/// receive rrc
	int ue_add_rrc_len = sizeof(ue_addr_rrc);
    r = recvfrom(udp_sock_rrc, buff, sizeof(buff), 0, (struct sockaddr*)&ue_addr_rrc, &ue_add_rrc_len);
    if (r == SOCKET_ERROR) {
        printf("recv failed\n");
        return;
    }
    buff[r] = '\0';
    printf("Received from ue RRC [%s:%d]: %s\n",
        inet_ntoa(ue_addr_rrc.sin_addr), ntohs(ue_addr_rrc.sin_port), buff);
       
	// gửi phản hồi cho UE
    const char* reply = "hello from server!";
    sendto(udp_sock, reply, strlen(reply), 0, (struct sockaddr*)&ue_addr, sizeof(ue_addr));
	sendto(udp_sock_rrc, reply, strlen(reply), 0, (struct sockaddr*)&ue_addr_rrc, sizeof(ue_addr_rrc));

    printf("done init gnb -------------------------------------\n");
}


bool send_mib(SOCKET socket, const MIB* mib, struct sockaddr_in* ue_addr) {
    //udp
    int ret;
    char buff[1024];
    memcpy(buff, mib, sizeof(MIB));
    ret = sendto(socket, buff, sizeof(MIB), 0, (struct sockaddr*)ue_addr, sizeof(*ue_addr));
    if (ret == SOCKET_ERROR) {
        printf("Error cant send data\n");
        return false;
    }
    // printf("Send data sucessfull\n");
    return true;
}

bool receive_ngap(SOCKET socket, NgAP* ngap) {
    //tcp rcv
    char buff[1024];
    if (recv(socket, buff, sizeof(NgAP), 0) == SOCKET_ERROR) {
        printf("receive NgAP failed\n");
        return false;
    }
    memcpy(ngap, buff, sizeof(NgAP));
    //printf("receive ngap from amf %d %d %d %d ", ngap->cn_domain, ngap->msg_type, ngap->tac, ngap->ue_id);

    return true;
}
bool send_rrc(SOCKET socket, const RRC* rrc, struct sockaddr_in* ue_addr) {
    //udp
    int ret;
    char buff[1024];
    memcpy(buff, rrc, sizeof(RRC));
    ret = sendto(socket, buff, sizeof(RRC), 0, (struct sockaddr*)ue_addr, sizeof(*ue_addr));
    if (ret == SOCKET_ERROR) {
        printf("Error cant send data\n");
        return false;
    }
    return true;
}
/// </funcion>  