#define _WINSOCK_DEPRECATED_NO_WARNINGS
#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <time.h>
#include <winsock2.h>
#include <ws2tcpip.h>

#define SCALE  10
#pragma comment(lib, "Ws2_32.lib")

/*
UDP client: socket() → {sendto() và recvfrom()} → closesocket()
10ms SFN tăng 1 đơn vị
Nếu chưa đồng bộ (ASYNC): 80ms cập nhật SFN 1 lần từ gNB
Nếu đã đồng bộ (SYNC): 800ms cập nhật SFN 1 lần từ gNB
*/

SOCKET ue_socket;
struct sockaddr_in gnb_addr;

typedef enum {
    ASYNC,
    SYNC
} State;

#pragma pack(push, 1)
typedef struct {
    uint8_t message_id;
    uint16_t sfn_value;
} MIB;
#pragma pack(pop)

typedef struct {
    unsigned int time;
    uint16_t sfn;
    State state;
} SystemParameter;

void increase_sfn(SystemParameter* s) {
    s->sfn = (s->sfn + 1) % 1024;
}
void updateSFN(SystemParameter* s, MIB* mib) {
    s->sfn = mib->sfn_value;
    printf(" --- update SFN = %d----- ", s->sfn);
}

void update_sfn(SystemParameter* s, uint16_t gnb_sfn) {
    if (gnb_sfn >= 0 && gnb_sfn <= 1023) {
        s->sfn = gnb_sfn;
        printf("Updated SFN to %d from gNB\n", s->sfn);
    }
    else {
        printf("Invalid gNB SFN\n");
    }
}




bool receive_mib(SOCKET socket, MIB* mib, struct sockaddr_in* gnb_addr) {
    int ret;
    int addr_len = sizeof(*gnb_addr);
    char buff[1024];
    ret = recvfrom(socket, buff, 1024, 0, (struct sockaddr*)&gnb_addr, &addr_len);
    if (ret == SOCKET_ERROR) {
        if (WSAGetLastError() == WSAETIMEDOUT) {
            printf("Time out\n");
        }
        else {
            printf("Error cant receive msg\n");
        }
        return false;
    }
    if (ret < sizeof(MIB)) {
        printf("size invalid :))), size: %d bytes\n", ret);
        return false;
    }
    memcpy(mib, buff, sizeof(MIB));


    //printf("Received MIB from gNB:\n");
    //printf("  message_id = %d\n", mib->message_id);
    //printf("  sfn_value  = %d\n", mib->sfn_value);    
    return true;
}
void initConnection() {
    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        printf("WSAStartup failed\n");
        return;
    }

    ue_socket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if (ue_socket == INVALID_SOCKET) {
        printf("Cannot create socket\n");
        WSACleanup();
        return;
    }
    printf("UDP ue start\n");
    // cấu hình địa chỉ của gNB
    gnb_addr.sin_family = AF_INET;
    gnb_addr.sin_port = htons(5500);
    gnb_addr.sin_addr.s_addr = inet_addr("127.0.0.1");


    // gửi dữ liệu tới gnb
    int gnb_addr_len = sizeof(gnb_addr);

    const char* msg = "hello from ue";
    int sent = sendto(ue_socket, msg, strlen(msg), 0, (struct sockaddr*)&gnb_addr, sizeof(gnb_addr));
    if (sent == SOCKET_ERROR) {
        printf("sent fail\n");
        closesocket(ue_socket);
        WSACleanup();
        return;
    }
    printf("sent hello from ue to gnb :)))\n");

    // nhận phản hồi từ gnb
    char buff[1024];
    int r = recvfrom(ue_socket, buff, sizeof(buff), 0, (struct sockaddr*)&gnb_addr,  &gnb_addr_len);
    if (r == SOCKET_ERROR) {
        printf("recvfrom() failed. Error Code: %d\n", WSAGetLastError());
    }
    else {
        buff[r] = '\0';
        printf("Reply from gnb: %s\n", buff);
    }
    printf("done init\n");

}



int main() {
    SystemParameter sp = { 0 };
    sp.time = 0;
    sp.sfn = 0;
    sp.state = ASYNC; // Bắt đầu ở trạng thái chưa đồng bộ

    MIB mib;

    initConnection();

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
            receive_mib(ue_socket, &mib, &gnb_addr);
            printf("receive MIB - M_ID = %d, SFN = %d", mib.message_id, mib.sfn_value);
        }

        if (sp.state == ASYNC && sp.time % 80 == 0) {
            updateSFN(&sp, &mib);
            sp.state = SYNC;// đặt UE ở trạng thái đã đồng bộ 
        }
        if(sp.state ==SYNC  && sp.time %800 ==0) updateSFN(&sp, &mib);

        printf("\n");
    }


    closesocket(ue_socket);
    WSACleanup();
    return 0;
}