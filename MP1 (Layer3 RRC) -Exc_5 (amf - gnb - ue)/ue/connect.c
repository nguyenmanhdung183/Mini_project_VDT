
#include"connect.h"

CRITICAL_SECTION sfn_lock;
volatile LONG increased_at_update_param = 0;
volatile LONG just_update_param = 0;

// return InterlockedCompareExchange(&increased, 0, 0); // đọc an toàn
//InterlockedExchange(&increased, 1);

SystemClock clk = { 0, 0, 0 };
State state = ASYNC;

int8_t table[5][4] = { // ánh xạ is và ns sang số sf
    { -1, -1, -1, -1 },//0
    {  9, -1, -1, -1 },//1
    {  4,  9, -1, -1 },//2
    {-1, -1, -1, -1},//3
    {  0,  4,  5,  9 }//4
};





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
	ue_socket_rrc = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if (ue_socket == INVALID_SOCKET || ue_socket_rrc == INVALID_SOCKET) {
        printf("Cannot create socket\n");
        WSACleanup();
        return;
    }
    printf("UDP ue start\n");

    // cấu hình địa chỉ của gNB
    gnb_addr.sin_family = AF_INET;
    gnb_addr.sin_port = htons(UDP_PORT);
    gnb_addr.sin_addr.s_addr = inet_addr("127.0.0.1");

    gnb_addr_rrc.sin_family = AF_INET;
    gnb_addr_rrc.sin_port = htons(UDP_PORT_RRC);
    gnb_addr_rrc.sin_addr.s_addr = inet_addr("127.0.0.1");

    Sleep(1000);

    // gửi dữ liệu tới gnb
    int gnb_addr_len = sizeof(gnb_addr);

    const char* msg = "hello from ue mib";
    int sent = sendto(ue_socket, msg, strlen(msg), 0, (struct sockaddr*)&gnb_addr, sizeof(gnb_addr));
    if (sent == SOCKET_ERROR) {
        printf("sent fail\n");
        closesocket(ue_socket);
        WSACleanup();
        return;
    }
    printf("sent hello from ue to gnb :)))\n");


	const char* msg_rrc = "hello from ue rrc";
	int sent_rrc = sendto(ue_socket_rrc, msg_rrc, strlen(msg_rrc), 0, (struct sockaddr*)&gnb_addr_rrc, sizeof(gnb_addr_rrc));
    if (sent_rrc == SOCKET_ERROR) {
        printf("sent fail rrc\n");
        closesocket(ue_socket_rrc);
        WSACleanup();
        return;
	}

    // nhận phản hồi từ gnb
    char buff[1024];
    int r = recvfrom(ue_socket, buff, sizeof(buff), 0, (struct sockaddr*)&gnb_addr, &gnb_addr_len);\
    if (r == SOCKET_ERROR) {
        printf("recv failed. Error Code: %d\n", WSAGetLastError());
    }
    else {
        buff[r] = '\0';
        printf("Reply from gnb: %s\n", buff);
    }

	r = recvfrom(ue_socket_rrc, buff, sizeof(buff), 0, (struct sockaddr*)&gnb_addr_rrc, &gnb_addr_len);
    if (r == SOCKET_ERROR) {
        printf("recv failed rrc. Error Code: %d\n", WSAGetLastError());
    }
    else {
        buff[r] = '\0';
        printf("Reply from gnb rrc: %s\n", buff);
	}
    printf("done init ue-------------------------------------\n");

}

bool receive_rrc(SOCKET socket, RRC* rrc, struct sockaddr_in* gnb_addr) {
    //udp recv
    int ret;
    int addr_len = sizeof(*gnb_addr);
    char buff[1024];
    ret = recvfrom(socket, buff, 1024, 0, (struct sockaddr*)gnb_addr, &addr_len);

    EnterCriticalSection(&sfn_lock);
    uint16_t sfn = clk.sfn;
    uint8_t sf = clk.sf;
    LeaveCriticalSection(&sfn_lock);

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
    if (rrc->ue_id != UE_ID) return false;

    if (check_correct_rrc_time(sfn, sf)) printf("[SFN=%d PO=%d]CORRECT TIME---", sfn, sf);
	else  printf("[SFN=%d PO=%d]WRONG TIME---", sfn, sf);


    printf("------RRC received from amf %d %d %d %d \n", rrc->cn_domain, rrc->msg_type, rrc->tac, rrc->ue_id);

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

unsigned __stdcall receive_mib_thread(void* arg) {
    MIB mib;
    static count = 0;
    while (1) {
        //InterlockedExchange(&just_update_param, 0); // nhảy sang cái khác rồi, set lại biến

        if (receive_mib(ue_socket, &mib, &gnb_addr)) count++;
        if (state = ASYNC) {//state == ASYNC
            EnterCriticalSection(&sfn_lock);
            clk.sfn = mib.sfn_value;
            clk.sf = 0;
            LeaveCriticalSection(&sfn_lock);
            InterlockedExchange(&just_update_param, 1);
            state = SYNC;
        }
        else if(count ==10){ // 800ms
            EnterCriticalSection(&sfn_lock);
            clk.sfn = mib.sfn_value;
            clk.sf = 0;
            LeaveCriticalSection(&sfn_lock);
            InterlockedExchange(&just_update_param, 1);
            count = 0;
        }
        
        
    }
}

unsigned __stdcall receive_rrc_thread(void* arg) {
    RRC rrc;
    while (1) {
        receive_rrc(ue_socket_rrc, &rrc, &gnb_addr_rrc);
    }
}


// ==== CLOCK ==== đếm thời gian
unsigned __stdcall clock_thread(void* arg) {
    LARGE_INTEGER freq, now, last;
    QueryPerformanceFrequency(&freq);
    QueryPerformanceCounter(&last);
    double elapsed;
    while (1) {
        QueryPerformanceCounter(&now);
        elapsed = (now.QuadPart - last.QuadPart) * 1000.0 / freq.QuadPart;

        if (elapsed >= INTERVAL_MS) {
            EnterCriticalSection(&sfn_lock);
            clk.time_ms += INTERVAL_MS;
            clk.sf = (clk.sf + 1) % 10; // 10 sfs per SFN (1ms each)
            if (clk.sf == 0) {
                clk.sfn = (clk.sfn + 1) % 1024;
				InterlockedExchange(&increased_at_update_param, 1); // đánh dấu đã tăng sfn
            }
            LeaveCriticalSection(&sfn_lock);
            last = now;
            if (InterlockedCompareExchange(&just_update_param, 0, 0)) {
                InterlockedExchange(&just_update_param, 0);
                if (!InterlockedCompareExchange(&increased_at_update_param, 0, 0)) {
                    InterlockedExchange(&just_update_param, 0);
                    EnterCriticalSection(&sfn_lock);
                    clk.time_ms += INTERVAL_MS;
                    clk.sf = (clk.sf + 1) % 10; // 10 sfs per SFN (1ms each)
                    if (clk.sf == 0) {
                        clk.sfn = (clk.sfn + 1) % 1024;
                    }
                    LeaveCriticalSection(&sfn_lock);

                }
            }
            InterlockedExchange(&increased_at_update_param, 0);

        }
    }
    return 0;
}


bool check_correct_rrc_time(uint16_t sfn, uint8_t sf) {
    // nếu mà sfn và sf thoả mãn mấy cái phương trình tính po. pf của UE thì return true;
    int a = PO;
	int b = PF;
    return (sfn % DRX == PF && sf == PO);
}
