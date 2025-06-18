#include"connect.h"

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
	//printf("sent ngap \n");
	printf("send ngap ueid = % d\n", ngap->ue_id);
	return true;
}
/// </funcion>