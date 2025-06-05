#include<stdio.h>
#include"global.h"
#include"connect.h"

/*
//giao tiếp với gnb bằng udp port 5000
đúng chu kỳ thức dậy đọc RRC
*/

int main() {
    printf("ue\n");

    SystemParameter sp = { 0, 0 };// lưu thông tin hệ thống
    RRC rrc; // nhận từ gnb
    MIB mib;// nhận từ gnb
    State state = ASYNC;
    udp_init();
    unsigned int sfn_mod_t = caculate_pf_mod_t(UE_ID);// PF % DRX = cái này

    //receive_rrc(ue_socket, &rrc, &gnb_addr);
    //receive_mib(ue_socket, &mib, &gnb_addr);

    /*
    đúng chu kỳ DRX thức để đọc bản tin RRC (160, 256, 512,...)
    nếu ko có bản tin paging từ GNB thì UE ngủ lại sau khi kiểm tra
    */
    while (1) {
        receive_rrc(ue_socket, &rrc, &gnb_addr);
    }
    

    closesocket(ue_socket);
    WSACleanup();
    system("pause");
    return 0;
}