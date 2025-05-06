#include<stdio.h>
#define _WINSOCK_DEPRECATED_NO_WARNINGS
#include<stdint.h>
#include<stdbool.h>
#include<winsock2.h>
#include <ws2tcpip.h>
#include<windows.h>

#pragma comment(lib, "Ws2_32.lib")
#define SCALE 10

#define UDP_PORT 5500
#define TCP_PORT 6000

#define UE_ID 123
#define DRX 128 //1280s
#define Ns 1 //số PO trong PF

/*
giao tiếp với amf bằng tcp port 6000
giao tiếp vớ ue bằng udp port 5000
nhận bản tin NgAP paging-> tính toán thời điểm thích hợp gửi RRC
*/

typedef struct {
	uint32_t msg_type;// 100 là paging
	uint32_t ue_id;
	uint32_t tac;// 100 là tac đúng
	uint32_t cn_domain;// 100 cho gọi thoại, 101 cho data
}NgAP;

typedef struct {
	uint32_t msg_type; // 100 là paging
	uint32_t ue_id;
	uint32_t tac;// 100 là tac đúng
	uint32_t cn_domain;// 100 cho gọi thoại, 101 cho data
}RRC;


typedef struct {
	uint8_t message_id;
	uint16_t sfn_value;
} MIB;
#pragma pack(pop)

typedef struct {
	unsigned int  time;
	uint16_t sfn;
} SystemParameter;

///<var>
SOCKET tcp_sock, udp_sock, amf_sock;
struct sockaddr_in gnb_addr_tcp, gnb_addr_udp, ue_addr, amf_addr;
///</var>

/// <funcion>
void udp_tcp_init();
bool receive_ngap(SOCKET socket, NgAP* ngap);
bool send_rrc(SOCKET socket, const RRC* rrc, struct sockaddr_in* ue_addr);
bool send_mib(SOCKET socket, const MIB* mib, struct sockaddr_in* ue_addr);

void increase_sfn(SystemParameter * sp){// coi như nhận ở slot 0
    sp->sfn = (sp->sfn + 1) % 1024;
}

uint16_t caculate_pf_mod_t(uint32_t ue_id) {
    int N = DRX / Ns;
    unsigned temp = (DRX / N) * (ue_id % N);
    printf("\npf_mod_t = %d\n", temp);
    return  temp;
}

void udp_tcp_init() {

    // khởi tạo wsadata tcp/udp
    WSADATA wsaData;

    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        printf("WSAStartup failed\n");
        return;
    }


    // tạo socket cho tcp/udp
    udp_sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    tcp_sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

    if (udp_sock == INVALID_SOCKET || tcp_sock == INVALID_SOCKET) {
        printf("Cannot create socket\n");
        WSACleanup();
        return;
    }

    //gán địa chỉ cho tcp  và udp
    gnb_addr_udp.sin_family = AF_INET;
    gnb_addr_udp.sin_port = htons(UDP_PORT);  // 5500
    gnb_addr_udp.sin_addr.s_addr = INADDR_ANY;

    gnb_addr_tcp.sin_family = AF_INET;
    gnb_addr_tcp.sin_port = htons(TCP_PORT);  // 6000
    gnb_addr_tcp.sin_addr.s_addr = INADDR_ANY;

    // Bind TCP
    if (bind(tcp_sock, (struct sockaddr*)&gnb_addr_tcp, sizeof(gnb_addr_tcp)) == SOCKET_ERROR) {
        printf("Bind TCP failed: %d\n", WSAGetLastError());
        closesocket(udp_sock);
        WSACleanup();
        return;
    }
    printf("TCP gNB start...\n");

    // Bind UDP 
    if (bind(udp_sock, (struct sockaddr*)&gnb_addr_udp, sizeof(gnb_addr_udp)) == SOCKET_ERROR) {
        printf("Bind UDP failed: %d\n", WSAGetLastError());
        closesocket(tcp_sock);
        closesocket(udp_sock);
        WSACleanup();
        return;
    }
    printf("UDP gNB start...\n");

    // Listen  TCP connection
    if (listen(tcp_sock, SOMAXCONN) == SOCKET_ERROR) {
        printf("listen AMF failed\n");
        closesocket(tcp_sock);
        closesocket(udp_sock);
        WSACleanup();
        return;
    }

    // Accept  AMF connection
    int amf_addr_size = sizeof(amf_addr);
    amf_sock = accept(tcp_sock, (struct sockaddr*)&amf_addr, &amf_addr_size);
    if (amf_sock == INVALID_SOCKET) {
        printf("accept AMF failed\n");
        closesocket(tcp_sock);
        closesocket(udp_sock);
        WSACleanup();
        return;
    }
    printf("amf connected.\n");

    //  UDP receive
    char buff[1024];
    int ue_addr_len = sizeof(ue_addr);
    memset(buff, 0, sizeof(buff));
    int r = recvfrom(udp_sock, buff, sizeof(buff), 0, (struct sockaddr*)&ue_addr, &ue_addr_len);
    if (r == SOCKET_ERROR) {
        printf("recv failed\n");
        return;
    }

    buff[r] = '\0';
    printf("Received from ue [%s:%d]: %s\n",
    inet_ntoa(ue_addr.sin_addr), ntohs(ue_addr.sin_port), buff);

    const char* reply = "hello from server!";
    sendto(udp_sock, reply, strlen(reply), 0, (struct sockaddr*)&ue_addr, sizeof(ue_addr));

    printf("done init gnb-------------------------------------\n");
}


bool send_mib(SOCKET socket, const MIB* mib, struct sockaddr_in* ue_addr) {
	//udp
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

bool receive_ngap(SOCKET socket, NgAP* ngap) {
	//tcp rcv
	char buff[1024];
	if (recv(socket, buff, sizeof(NgAP), 0) == SOCKET_ERROR) {
		printf("receive NgAP failed\n");
		return false;
	}
	memcpy(ngap, buff, sizeof(NgAP));
    printf("receive ngap from amf %d %d %d %d ", ngap->cn_domain, ngap->msg_type, ngap->tac, ngap->ue_id);

	return true;
}
bool send_rrc(SOCKET socket, const RRC* rrc, struct sockaddr_in* ue_addr) {
	//udp
	int ret;
	char buff[1024];
	memcpy(buff, rrc, sizeof(RRC));
	ret = sendto(socket, buff, sizeof(RRC), 0, (struct sockaddr*)ue_addr, sizeof(*ue_addr));
	if (ret == SOCKET_ERROR) {
		printf("Error cant send data\n");
		return false;
	}
	return true;
}
/// </funcion>


// hàm trong luồng nhận amf tcp từ amf
CRITICAL_SECTION cs;// tránh race condition
bool ready = false;
DWORD WINAPI listening_amf(LPVOID lpParam) {
    NgAP* ngap = (NgAP*)lpParam;
    while (1) {
        if (receive_ngap(amf_sock, ngap)) {
            EnterCriticalSection(&cs);
            ready = true;
            printf("---------------------------------recvei NgAP\n");
            LeaveCriticalSection(&cs);
        }
    }
    return 0;
}

int main() {
	printf("gnb\n");

    SystemParameter sp = {0, 0};// lưu thông tin hệ thống
	NgAP ngap;// nhận dc từ amf
	RRC rrc; //nhận từ amf và gửi cho ue
    RRC rrc0 = {0};// rrc ko chứa thông tin
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

    while (1) {
        Sleep(10 * SCALE);
        sp.time += 10;
        if (sp.time % 10 == 0) {
            increase_sfn(&sp);

        }
        printf("SFN gnb now = %d ---", sp.sfn);

        if (sp.time % 80 == 0) {
            mib.message_id = mib.message_id+1;
            mib.sfn_value = sp.sfn;
            send_mib(udp_sock, &mib, &ue_addr);
            printf("send MIB - M_ID = %d, SFN = %d", mib.message_id, mib.sfn_value);

        }
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