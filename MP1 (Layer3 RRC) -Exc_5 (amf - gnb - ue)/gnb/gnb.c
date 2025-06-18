#include<stdio.h>
#include <stdint.h>
#include"global.h"
#include"connect.h"
#include"caculate.h"
#include"queue.h"
#include"hashmap.h"
#include<windows.h>


/*
giao tiếp với amf bằng tcp port 6000
giao tiếp vớ ue bằng udp port 5000
nhận bản tin NgAP paging-> tính toán thời điểm thích hợp gửi RRC
*/


int main() {
    printf("gnb\n");
    InitializeCriticalSection(&queue_lock);
    InitializeCriticalSection(&sfn_lock);
    InitializeCriticalSection(&map_cs);

    udp_tcp_init();

    SystemParameter sp = { 0, 0 };// lưu thông tin hệ thống
    NgAP ngap;// nhận dc từ amf
    RRC rrc; //nhận từ amf và gửi cho ue
    RRC rrc0 = { 0 };// rrc ko chứa thông tin
    MIB mib = { 135, 357 };// gửi cho ue
    bool is_sent_ngap_2_ue = false;


    receive_ngap(amf_sock, &ngap);
    memcpy(&rrc, &ngap, sizeof(RRC));
   // unsigned int sfn_mod_t = caculate_pf_mod_t(ngap.ue_id);// PF % DRX = cái này


    _beginthreadex(NULL, 0, clock_thread, NULL, 0, NULL);// đếm thời gian
    _beginthreadex(NULL, 0, broadcast_mib_thread, NULL, 0, NULL);// gửi mib cho ue
	_beginthreadex(NULL, 0, listening_amf, NULL, 0, NULL);// nhận ngap từ amf
	for (int i = 0; i < 1; i++) _beginthreadex(NULL, 0, scheduler, NULL, 0, NULL);// lên lịch gửi RRC paging
    for (int i = 0; i < 1; i++) _beginthreadex(NULL, 0, worker, NULL, 0, NULL);//tìm kiếm RRC paging và gửi
    _beginthreadex(NULL, 0, log_thread, NULL, 0, NULL);

    while(1){}

    closesocket(tcp_sock);
    closesocket(udp_sock);
    WSACleanup();
    system("pause");
    return 0;
}