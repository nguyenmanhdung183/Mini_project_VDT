#define _WINSOCK_DEPRECATED_NO_WARNINGS
#include<stdio.h>
#include<stdint.h>
#include<stdbool.h>
#include<winsock2.h>
#include<time.h>
#include <ws2tcpip.h>
#pragma comment(lib, "Ws2_32.lib")
#define SCALE 10

#define TCP_PORT 6000

/*
giao tiếp với gnb bằng tcp port 6000
gửi bản tin NgAP paging
*/
///<var>
WSADATA wsaData;
SOCKET tcp_sock;
struct sockaddr_in gnb_addr;
///</var>

typedef struct {
	uint32_t msg_type;// 100 là paging
	uint32_t ue_id;
	uint32_t tac;// 100 là tac đúng
	uint32_t cn_domain;// 100 cho gọi thoại, 101 cho data
}NgAP;

/// <fucion>
void tcp_init(); 
bool send_ngap(SOCKET sock, const  NgAP ngap);

void tcp_init() {
	WSADATA wsaData;

	if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
		printf("WSAStartup failed\n");
		return;
	}
	tcp_sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

	if (tcp_sock == INVALID_SOCKET) {
		printf("Cannot create socket\n");
		WSACleanup();
		return;
	}
	gnb_addr.sin_family = AF_INET;
	gnb_addr.sin_port = htons(TCP_PORT);
	//gnb_addr.sin_addr.s_addr = inet_addr("127.0.0.1");
	inet_pton(AF_INET, "127.0.0.1", &gnb_addr.sin_addr);

	Sleep(500);

	if (connect(tcp_sock, (struct sockaddr*)&gnb_addr, sizeof(gnb_addr)) == SOCKET_ERROR) {
		printf("connect failed\n");
		closesocket(tcp_sock);
		WSACleanup();
		return;
	}
	printf("connect to gnb\n");
	printf("done init amf-------------------------------------\n");
}
bool send_ngap(SOCKET sock, const  NgAP* ngap) {
	char buff[1024];
	memcpy(buff, ngap, sizeof(NgAP));
	if (send(sock, buff, sizeof(NgAP), 0) == SOCKET_ERROR) {
		printf("sent ngap failed\n");
		return false;
	}
	printf("sent ngap\n");
	return true;
}
/// </funcion>

int main() {
	printf("amf\n");
	NgAP ngap = { 100,123,100,101};// bản tin báo có gọi video
	tcp_init();
	send_ngap(tcp_sock, &ngap);
	// gửi NgAP cho gnb
	/*
	giả sử cứ Xms(random) thì sẽ có 1 yêu cầu đến ue
	-> gửi ngap cho ue
	*/

	send_ngap(tcp_sock, &ngap);

	Sleep(30000);
	send_ngap(tcp_sock, &ngap);

	Sleep(123456);
	send_ngap(tcp_sock, &ngap);

	Sleep(1234567);
	send_ngap(tcp_sock, &ngap);

	Sleep(200000);
	send_ngap(tcp_sock, &ngap);

	printf("\nend\n");
	system("pause");
	closesocket(tcp_sock);
	WSACleanup();
	return 0;
}