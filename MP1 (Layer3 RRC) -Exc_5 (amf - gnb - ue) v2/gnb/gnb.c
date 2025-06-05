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
    unsigned int sfn_mod_t = caculate_pf_mod_t(ngap.ue_id);// PF % DRX = cái này

    //send_rrc(udp_sock, &rrc, &ue_addr);
    //send_mib(udp_sock, &mib, &ue_addr);

    /*
    luồng nhận ngap-> nếu nhận được ngap -> gửi RRC đến cho UE
    kiểm tra xem TA trong bản tin có trong vùng phủ của mình ko
    nếu có thì sẽ tạo RRC paging và tính toán thời gian
    gửi qua kênh PCH tới tất cả UE
    nếu ko có yêu cầu paging từ amf thì sẽ ko gửi gì cho ue


    GNodeB tính toán SFN, PF, PO dựa vào UE_ID.
    Gửi bản tin RRC đúng vào thời điểm phù hợp.
    */

    // cứ 10 ms tăng SFN, 80 ms gửi mib cho ue -> luồng gửi  mib
    // giả sử cứ 2340ms thì kiểm tra ready rồi gửi rcc cho ue
    // nhận 400-500 bản tin ngap 1s để xử lý -> đẩy vào 1 hàng đợi để lên lịch, thằng nào có lịch sớm hơn thì được đẩy ra trước
    /*
    uint32_t count = 0;
    DWORD64 start;
    while (1) {
        if (receive_ngap(amf_sock, &ngap)) {
			if (count == 0) start = GetTickCount64();
            if (!ngap.cn_domain && !ngap.msg_type && !ngap.tac && !ngap.ue_id) break;
			count++;
       }
		//Sleep(1); // tránh quá tải CPU
    }
    DWORD64 end = GetTickCount64(); // Thời gian kết thúc (ms)	long seconds = end.tv_sec - start.tv_sec;
    DWORD64 elapsed_ms = end - start;

	printf("received %d NgAP messages\n", count);

    printf("Runtime: %llu ms\n", elapsed_ms);
    if (elapsed_ms > 0)
        printf("Speed: %.2f NgAP/s\n", (count * 1000.0) / elapsed_ms);
    else
        printf("Thời gian quá ngắn để tính toán\n");
    
    */


    _beginthreadex(NULL, 0, clock_thread, NULL, 0, NULL);// đếm thời gian
	_beginthreadex(NULL, 0, listening_amf, NULL, 0, NULL);// nhận ngap từ amf
	for (int i = 0; i < 1; i++) _beginthreadex(NULL, 0, scheduler, NULL, 0, NULL);// lên lịch gửi RRC paging
    for (int i = 0; i < 1; i++) _beginthreadex(NULL, 0, worker, NULL, 0, NULL);//tìm kiếm RRC paging và gửi
    _beginthreadex(NULL, 0, log_thread, NULL, 0, NULL);
    
    while (1) {
		// tránh end các thread đang chạy :))))
    }

    closesocket(tcp_sock);
    closesocket(udp_sock);
    WSACleanup();
    system("pause");
    return 0;
}