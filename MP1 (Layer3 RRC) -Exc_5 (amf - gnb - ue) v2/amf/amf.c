#include<stdio.h>
#define _WINSOCK_DEPRECATED_NO_WARNINGS

#include"global.h"
#include"connect.h"
#include<time.h>
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

	DWORD64 start = GetTickCount64(); // Thời gian bắt đầu (ms)

	while (1) {

		for (i = 0; i < numberOfNgapPerSecond; i++) {
			// create random ue_id :)))))
			ngap_test.ue_id = 1 + rand() % (999);
			if (i%10 ==0) send_ngap(tcp_sock, &ngap);
			else send_ngap(tcp_sock, &ngap);
			Sleep(100);
		}
	

		}
		NgAP ngap_test_n1;
		ngap_test_n1.cn_domain = 0;
		ngap_test_n1.tac = 0;
		ngap_test_n1.msg_type = 0;
		ngap_test_n1.ue_id = 0;
		send_ngap(tcp_sock, &ngap_test_n1);
	
	DWORD64 end = GetTickCount64(); // Thời gian kết thúc (ms)	long seconds = end.tv_sec - start.tv_sec;
	DWORD64 elapsed_ms = end - start;

	printf("Runtime: %llu ms\n", elapsed_ms);
	if (elapsed_ms > 0)
		printf("Speed: %.2f NgAP/s\n", (numberOfNgapPerSecond * 1000.0) / elapsed_ms);
	else
		printf("Thời gian quá ngắn để tính toán\n");
	printf("\nend\n");
	system("pause");
	closesocket(tcp_sock);
	WSACleanup();
	return 0;
}