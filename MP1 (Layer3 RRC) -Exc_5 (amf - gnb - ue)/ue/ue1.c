#include<stdio.h>
#include"global.h"
#include"connect.h"


extern CRITICAL_SECTION sfn_lock;
extern SystemClock clk;
extern State state;


int main() {
    printf("ue\n");

    SystemParameter sp = { 0, 0 };// lưu thông tin hệ thống
    udp_init();
    unsigned int sfn_mod_t = caculate_pf_mod_t(UE_ID);// PF % DRX = cái này
    InitializeCriticalSection(&sfn_lock);


    /*
    đúng chu kỳ DRX thức để đọc bản tin RRC (160, 256, 512,...)
    nếu ko có bản tin paging từ GNB thì UE ngủ lại sau khi kiểm tra
    */
    _beginthreadex(NULL, 0, clock_thread, NULL, 0, NULL);// đếm thời gian
    _beginthreadex(NULL, 0, receive_mib_thread, NULL, 0, NULL);// đếm thời gian
    _beginthreadex(NULL, 0, receive_rrc_thread, NULL, 0, NULL);// đếm thời gian

    while(1){}

    closesocket(ue_socket);
    closesocket(ue_socket_rrc);
    WSACleanup();
    system("pause");
    return 0;
}