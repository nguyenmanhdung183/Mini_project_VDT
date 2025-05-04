#include <stdio.h>
#define _WINSOCK_DEPRECATED_NO_WARNINGS 
#include <stdlib.h>
#include <stdint.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <string.h>
#include <stdbool.h>
#include <time.h>
#pragma comment(lib, "Ws2_32.lib")

/*
UDP server : socket() → bind() → {recvfrom() và sendto()} → closesocket()
10ms SFN tăng 1 đơn vị
80ms gNB gửi MIB chứa SFN hiện tại cho UE
*/

#define SCALE  10


SOCKET gnb_socket;
struct sockaddr_in gnb_addr, ue_addr;

#pragma pack(push, 1)
typedef struct {
    uint8_t message_id;
    uint16_t sfn_value;
} MIB;
#pragma pack(pop)

typedef struct {
    unsigned int  time;
    uint16_t sfn;
} SystemParameter;

void increase_sfn(SystemParameter* s) {
    s->sfn = (s->sfn +1) % 1024;
}



bool send_mib(SOCKET socket, const MIB* mib, struct sockaddr_in* ue_addr) {
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

void initConnection() {
    // khoi tao winsock
    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        printf("WSAStartup failed\n");
        return;
    }
    // khoi tao socket
    gnb_socket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if (gnb_socket == INVALID_SOCKET) {
        printf("Cannot create socket\n");
        WSACleanup();
        return;
    }
    // bind add 2 socket
    gnb_addr.sin_family = AF_INET;
    gnb_addr.sin_port = htons(5500);
    gnb_addr.sin_addr.s_addr = INADDR_ANY;// chấp nhận kết nối từ mọi IP
    //inet_pton(AF_INET, "127.0.0.1", &gnb_addr.sin_addr);

    if (bind(gnb_socket, (struct sockaddr*)&gnb_addr, sizeof(gnb_addr)) == SOCKET_ERROR) {
        printf("Bind failed: %d\n", WSAGetLastError());
        closesocket(gnb_socket);
        WSACleanup();
        return;
    }
    printf("UDP gNB start...\n");

    // nhận dữ liệu & phản hồi
    char buff[1024];
    int ue_addr_len = sizeof(ue_addr);
  //  while (1) {
        memset(buff, 0, sizeof(buff));
        int r = recvfrom(gnb_socket, buff, sizeof(buff), 0, (struct sockaddr*)&ue_addr, &ue_addr_len);
        if (r == SOCKET_ERROR) {
            printf("recv faile\n");
            //break;
            return;
        }

        buff[r] = '\0';
        printf("Received from ue [%s:%d]: %s\n",
            inet_ntoa(ue_addr.sin_addr), ntohs(ue_addr.sin_port), buff);
        // gửi phản hồi
        const char* reply = "hello from server !";
        sendto(gnb_socket, reply, strlen(reply), 0, (struct sockaddr*)&ue_addr, sizeof(ue_addr));

        printf("done init\n");
    //}
}



int main() {
    SystemParameter sp = { 0 };
    sp.time = 0;
    sp.sfn = 567;

    MIB mib;
    mib.message_id = 123;
    mib.sfn_value = sp.sfn;

    initConnection();
    //send_mib(gnb_socket, &mib, &ue_addr);

    /* 
    giả sử hệ thống sau khi sleep 10ms * SCALE(để cho dễ quan sát log nên tăng thời gian 10ms lên gấp (SCALE) lần)
    thì sẽ tăng biến đếm sp.time lên 10 (ms) 
    */
    int sleep_interval_after_scale = 10 * SCALE;
    while (1) {
        Sleep(sleep_interval_after_scale);
        sp.time += 10;
        if (sp.time % 10 == 0) {
            increase_sfn(&sp);
        }
        printf("SFN now = %d ---", sp.sfn);
        

        if (sp.time % 80 == 0) {
            mib.message_id++;
            mib.sfn_value = sp.sfn;
            send_mib(gnb_socket, &mib, &ue_addr);
            printf("send MIB - M_ID = %d, SFN = %d", mib.message_id, mib.sfn_value);
        }


        printf("\n");
    }



    closesocket(gnb_socket);
    WSACleanup();
    return 0;
}