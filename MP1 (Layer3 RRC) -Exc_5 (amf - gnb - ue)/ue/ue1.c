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

    // nhận mib và cập nhập sp
    while (1) {
        Sleep(10 * SCALE);
        sp.time += 10;
        if (sp.time % 10 == 0) {
            increase_sfn(&sp);
            printf("SFN ue now = %d ---", sp.sfn);
        }

        if (sp.time % 80 == 0) {
            receive_mib(ue_socket, &mib, &gnb_addr);
            printf("receive MIB - M_ID = %d, SFN = %d", mib.message_id, mib.sfn_value);
        }

        if (state == ASYNC && sp.time % 80 == 0) {
            updateSFN(&sp, &mib);
            state = SYNC;// đặt UE ở trạng thái đã đồng bộ 
        }

        if (state == SYNC && sp.time % 800 == 0) updateSFN(&sp, &mib);
        printf("\n");

        // nhận rrc
        if (sp.sfn % DRX == sfn_mod_t) {// có thể có hoặc ko có dữ liệu từ rrc
            receive_rrc(ue_socket, &rrc, &gnb_addr);
        }
    }

    closesocket(ue_socket);
    WSACleanup();
    system("pause");
    return 0;
}