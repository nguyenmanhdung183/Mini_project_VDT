#include<stdio.h>
#define _WINSOCK_DEPRECATED_NO_WARNINGS

#include"global.h"
#include"connect.h"

/*
giao tiếp với gnb bằng tcp port 6000
gửi bản tin NgAP paging
*/


/// <fucion>



int main() {
	printf("amf\n");
	NgAP ngap = { 100,123,100,101 };// bản tin báo có gọi video
	tcp_init();

	// gửi 400-500 bản tin ngap 1 s

	// gửi NgAP cho gnb
	/*
	giả sử cứ Xms(random) thì sẽ có 1 yêu cầu đến ue
	-> gửi ngap cho ue
	*/

#if 0
	send_ngap(tcp_sock, &ngap);

	send_ngap(tcp_sock, &ngap);

	Sleep(30000);
	send_ngap(tcp_sock, &ngap);

	Sleep(123456);
	send_ngap(tcp_sock, &ngap);

	Sleep(1234567);
	send_ngap(tcp_sock, &ngap);

	Sleep(200000);
	send_ngap(tcp_sock, &ngap);
#endif
	int i = 0;
	//int ue_id; //int r = min + rand() % (max - min + 1);
	NgAP ngap_test;
	ngap_test.cn_domain = 100;
	ngap_test.tac = 100;
	ngap_test.msg_type = 100;
	while (1) {
		for (i = 0; i < numberOfNgapPerSecond; i++) {
			// create random ue_id :)))))
			ngap_test.ue_id = 1 + rand() % (999);
			if (i == 1 || i==100) send_ngap(tcp_sock, &ngap);
			else send_ngap(tcp_sock, &ngap_test);
			Sleep(1000);

		}
	}
	printf("\nend\n");
	system("pause");
	closesocket(tcp_sock);
	WSACleanup();
	return 0;
}