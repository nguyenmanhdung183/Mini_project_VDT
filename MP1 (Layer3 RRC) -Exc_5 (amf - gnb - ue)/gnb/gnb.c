#include<stdio.h>
#include <stdint.h>
#include"queue.h"
#include"global.h"
#include"connect.h"
#include<windows.h>


/*
giao tiếp với amf bằng tcp port 6000
giao tiếp vớ ue bằng udp port 5000
nhận bản tin NgAP paging-> tính toán thời điểm thích hợp gửi RRC
*/



// hàm trong luồng nhận amf tcp từ amf
CRITICAL_SECTION cs;// tránh race condition
bool ready = false;
DWORD WINAPI listening_amf(LPVOID lpParam) {
    NgAP* ngap = (NgAP*)lpParam;
    while (1) {
        if (receive_ngap(amf_sock, ngap)) {
            EnterCriticalSection(&cs);
            ready = true;
            printf("----------------------------recvei NgAP\n");
            LeaveCriticalSection(&cs);
        }
    }
    return 0;
}

int main() {
    printf("gnb\n");

    SystemParameter sp = { 0, 0 };// lưu thông tin hệ thống
    NgAP ngap;// nhận dc từ amf
    RRC rrc; //nhận từ amf và gửi cho ue
    RRC rrc0 = { 0 };// rrc ko chứa thông tin
    MIB mib = { 135, 357 };// gửi cho ue
    bool is_sent_ngap_2_ue = false;

    udp_tcp_init();

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

    // nghe tcp
    InitializeCriticalSection(&cs);
    HANDLE  hThread = CreateThread(
        NULL,              // Không cần bảo mật
        0,                 // Kích thước stack mặc định
        listening_amf,
        (LPVOID)&ngap,
        0,
        NULL               //thread ID
    );

    // cứ 10 ms tăng SFN, 80 ms gửi mib cho ue -> luồng gửi  mib
    // giả sử cứ 2340ms thì kiểm tra ready rồi gửi rcc cho ue
    // nhận 400-500 bản tin ngap 1s để xử lý -> đẩy vào 1 hàng đợi để lên lịch, thằng nào có lịch sớm hơn thì được đẩy ra trước

    while (1) {
        Sleep(10 * SCALE);
        sp.time += 10;
        if (sp.time % 10 == 0) {
            increase_sfn(&sp);

        }
        printf("SFN gnb now = %d ---", sp.sfn);

        if (sp.time % 80 == 0) {
            mib.message_id = mib.message_id + 1;
            mib.sfn_value = sp.sfn;
            send_mib(udp_sock, &mib, &ue_addr);
            printf("send MIB - M_ID = %d, SFN = %d", mib.message_id, mib.sfn_value);

        }
        /*
        ở đoạn thread nhận NgAP từ AMF, cần đẩy vào queue (ví dụ vậy)
        Xử lý sao cho đoạn dưới này có thể gửi đúng lúc cho UE
        */
        if (sp.sfn % DRX == sfn_mod_t) {// gửi rrc  -> sp.time % 2340 == 0
            EnterCriticalSection(&cs);  // Bắt đầu vùng bảo vệ
            if (ready) {
                printf("ready change - update from amf\n");
                memcpy(&rrc, &ngap, sizeof(NgAP));
                ready = false;
                //is_sent_ngap_2_ue = false;
                if (send_rrc(udp_sock, &rrc, &ue_addr)) { //-> send_rrc(udp_sock, &rrc, &ue_addr)
                    printf("send RRC to ue\n");
                    printf(" ");
                }
            }
            else {
                send_rrc(udp_sock, &rrc0, &ue_addr);
            }
            LeaveCriticalSection(&cs);  // Kết thúc vùng bảo vệ nếu chưa nhận tín hiệu
        }

        printf("\n");

    }

    DeleteCriticalSection(&cs);
    closesocket(tcp_sock);
    closesocket(udp_sock);
    WSACleanup();
    system("pause");
    return 0;
}