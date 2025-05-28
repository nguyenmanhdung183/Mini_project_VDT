
#include"connect.h"



void increase_sfn(SystemParameter* sp) {
    sp->sfn = (sp->sfn + 1) % 1024;
}

uint16_t caculate_pf_mod_t(uint32_t ue_id) {
    int N = DRX / Ns;
    unsigned temp = (DRX / N) * (ue_id % N);
    printf("\npf_mod_t = %d\n", temp);
    return  temp;
}

void updateSFN(SystemParameter* s, MIB* mib) {
    s->sfn = mib->sfn_value;
    printf(" --- update SFN = %d- ", s->sfn);
}
void udp_init() {
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
    gnb_addr.sin_port = htons(UDP_PORT);
    gnb_addr.sin_addr.s_addr = inet_addr("127.0.0.1");

    Sleep(1000);

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
    int r = recvfrom(ue_socket, buff, sizeof(buff), 0, (struct sockaddr*)&gnb_addr, &gnb_addr_len);
    if (r == SOCKET_ERROR) {
        printf("recv failed. Error Code: %d\n", WSAGetLastError());
    }
    else {
        buff[r] = '\0';
        printf("Reply from gnb: %s\n", buff);
    }
    printf("done init ue-------------------------------------\n");

}

bool receive_rrc(SOCKET socket, RRC* rrc, struct sockaddr_in* gnb_addr) {
    //udp recv
    int ret;
    int addr_len = sizeof(*gnb_addr);
    char buff[1024];
    ret = recvfrom(socket, buff, 1024, 0, (struct sockaddr*)gnb_addr, &addr_len);
    if (ret < 0) {
        if (errno == EWOULDBLOCK || errno == EAGAIN) {
            printf("Không có dữ liệu – thoát không blocking.\n");
        }
        else {
            perror("recvfrom lỗi");
        }
    }
    if (ret == SOCKET_ERROR) {
        if (WSAGetLastError() == WSAETIMEDOUT) {
            printf("Time out\n");
        }
        else {
            printf("Error cant receive RRC\n");
        }
        return false;
    }
    if (ret < sizeof(RRC)) {
        printf("size invalid :))), size: %d bytes\n", ret);
        return false;
    }
    memcpy(rrc, buff, sizeof(RRC));
    printf("---------------------------------RRC received from amf %d %d %d %d \n", rrc->cn_domain, rrc->msg_type, rrc->tac, rrc->ue_id);

    return true;
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
            printf("Error cant receive MIB\n");
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
   // printf("\nreceice mib from gnb %d %d", mib->message_id, mib->sfn_value);
    return true;
}