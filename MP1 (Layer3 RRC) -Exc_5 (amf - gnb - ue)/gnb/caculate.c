#include "hashmap.h"
#include "caculate.h"
#include"global.h"
#include <math.h>
#include"queue.h"
extern CRITICAL_SECTION queue_lock;
extern CRITICAL_SECTION sfn_lock;

SystemClock clk = { 0, 0, 0 };
volatile long rrc_sent = 0;


int8_t table[5][4] = { // ánh xạ is và ns sang số sf
    { -1, -1, -1, -1 },//0
    {  9, -1, -1, -1 },//1
    {  4,  9, -1, -1 },//2
    {-1, -1, -1, -1},//3
    {  0,  4,  5,  9 }//4
};



void increase_sfn(SystemParameter* sp) {// coi như nhận ở sf 0
    sp->sfn = (sp->sfn + 1) % 1024;
}

uint16_t caculate_pf_mod_t(uint32_t ue_id) {
	uint8_t N = DRX / Ns; // số lượng paging frame trong DRX cycle
    unsigned temp = (DRX / N) * (ue_id % N);
    printf("\npf_mod_t = %d\n", temp);
    return  temp;
}



// ==== CALC PF & PO ==== khi có ngap thì đẩy vào đây để cho ra paging task rồi đẩy vào queue 
//PagingTask calc_task(NgAP ngap) {
//    PagingTask task;
//    task.ngap = ngap;
//    task.PF = (DRX / Ns) * (ngap.ue_id % (DRX / Ns));
//    task.PO = (ngap.ue_id / (DRX / Ns)) % Ns;
//    return task;
//}


// ==== CLOCK ==== đếm thời gian
unsigned __stdcall clock_thread(void* arg) {
    LARGE_INTEGER freq, now, last;
    QueryPerformanceFrequency(&freq);
    QueryPerformanceCounter(&last);

    while (1) {
        QueryPerformanceCounter(&now);
        double elapsed = (now.QuadPart - last.QuadPart) * 1000.0 / freq.QuadPart;

        if (elapsed >= INTERVAL_MS) {
            EnterCriticalSection(&sfn_lock);
            clk.time_ms += INTERVAL_MS;
            clk.sf = (clk.sf + 1) % 10; // 10 sfs per SFN (1ms each)
            if (clk.sf == 0) clk.sfn = (clk.sfn + 1) % 1024;
            LeaveCriticalSection(&sfn_lock);
            last = now;
        }
    }
    return 0;
}

// ==== WORKER ==== kiểm tra queue và gửi ngap đi
/*
unsigned __stdcall worker(void* arg) {
    NgAP ngap;
    while (1) {
        EnterCriticalSection(&sfn_lock);
        uint16_t sfn = clk.sfn;
        uint8_t sf = clk.sf;
        LeaveCriticalSection(&sfn_lock);

        if (dequeue(&task, sfn, sf)) {
            // thực hiện rrc (copy từ ngap sang rrc) cho UE, broadcast mới chuẩn :))))
            RRC rrc;
            memcpy(&rrc, &task.ngap, sizeof(NgAP));
            if (rrc.ue_id == 123) {
                send_rrc(udp_sock, &rrc, &ue_addr);
                printf("sent rrc to UE------------------\n");

            }
            InterlockedIncrement(&rrc_sent);
        }
    }
    return 0;
}
*/

// ==== LISTENING AMF ==== nhận ngap từ amf và đẩy vào queue
unsigned __stdcall listening_amf(void* arg) {
    NgAP ngap;
    while (1) {
        if (receive_ngap(amf_sock, &ngap)) {
            enqueue(ngap);
           // printf("------------------received NgAP\n");
        }
    }
}

// ====WORKER ==== kiểm tra map và gửi rrc đi :))))
unsigned __stdcall worker(void* arg) {

    while (1) {
        RRC rrc;
        EnterCriticalSection(&sfn_lock);
        uint16_t sfn = clk.sfn;
        uint8_t sf = clk.sf;
        LeaveCriticalSection(&sfn_lock);

		uint32_t key = sfn * 10 + sf; // key = PF * 1024 + PO
		Vector* vec = map_get(key);
        if (vec != NULL) { // nếu có key
            for (size_t i = 0; i < vec->size; i++) {
		// chuẩn là phải broadcast cho tất cả các UE trong vec, nhưng ở đây chỉ gửi cho UE có ID = 123 để test
                memcpy(&rrc, &vec->items[i], sizeof(RRC));
                if (rrc.ue_id == UE_ID) { // chỉ gửi cho UE có ID = 123
                    send_rrc(udp_sock_rrc, &rrc, &ue_addr_rrc);
                   // printf("sent rrc to UE------------------\n");
                }
                InterlockedIncrement(&rrc_sent);
            }
			// xoá cái entry trong hash map sau khi đã gửi hết
			map_remove(key);
		}
	}//end while

}

unsigned __stdcall log_thread(void* arg) {//hàm nay chưa chuẩn lắm, chỉ để test thôi
    while (1) {
        Sleep(1000);
        long count = InterlockedExchange(&rrc_sent, 0);
        printf("[SFN=%d] Paging sent in last second: %ld\n", clk.sfn, count);
    }
    return 0;
}



// ==== SCHEDULER ====
unsigned __stdcall scheduler(void* arg) {
	NgAP ngap;
    RRC rrc;
	RrcParmeter rrc_param;
    while (1) {
        EnterCriticalSection(&sfn_lock);
        uint16_t sfn = clk.sfn;
        uint8_t sf = clk.sf;
		LeaveCriticalSection(&sfn_lock);

        if (dequeue(&ngap)) {
			// kiểm tra sfn và sf hiện tại, tính toán PF và PO cho ngap
            // đẩy vào hash table với key = PF*1024 + PO
			memcpy(&rrc, &ngap, sizeof(NgAP));
			//printf("schedule NgAP: UE ID = %d, SFN = %d, SF = %d\n", rrc.ue_id, sfn, sf);
			rrc_param = calculate_rrc_parameter(ngap, sfn, sf);
			map_insert(rrc_param.PF * 10 + rrc_param.PO, rrc);

			//printf("map insert : PF = %d, PO = %d, UE ID = %d\n", rrc_param.PF, rrc_param.PO, rrc.ue_id);

        }
    }
    return 0;
}

RrcParmeter calculate_rrc_parameter(NgAP ngap,uint16_t sfn, uint8_t sf ) {
    
    uint8_t N = DRX / Ns; // số lượng paging frame trong DRX cycle
    RrcParmeter rrc_param;
    uint16_t PF = (DRX / N) * (ngap.ue_id % (N));
       
    rrc_param.is = ((uint8_t)floor((double)ngap.ue_id / N)) % Ns;
    rrc_param.PO = table[Ns][rrc_param.is];

  //  if (sfn % DRX >= PF && sf > rrc_param.PO) {
  //      // lên lịch ở PF sau
		//rrc_param.PF = ((sfn / DRX) + 1) * DRX + PF;
  //  }
  //  else {
		////lên lịch ở PF hiện tại
  //      rrc_param.PF = sfn;
  //  }
    if (sfn % DRX == PF) {
        if(rrc_param.PO > sf) {
            rrc_param.PF = sfn; // lên lịch ở PF hiện tại
        }
        else {
            rrc_param.PF = ((uint8_t)floor(sfn / DRX) + 1) * DRX + PF; // lên lịch ở PF sau
        }
    }
    else {
		rrc_param.PF = ((uint8_t)floor(sfn / DRX) + 1) * DRX + PF; // lên lịch ở PF sau
    }
    return rrc_param;
}


unsigned __stdcall broadcast_mib_thread(void* arg) {
	// cứ 80ms broadcast mib cho tất cả các UE chứa sfn hiện tại
    MIB mib;
    mib.message_id = 1;
    uint16_t sfn;
     uint8_t sf;
    while (1) {
        EnterCriticalSection(&sfn_lock);
        sfn = clk.sfn;
        sf = clk.sf;
        LeaveCriticalSection(&sfn_lock);
        // mới chỉ gửi cho 1 ue thôi
        if (sfn % 8 == 0 && sf == 0) {
            mib.sfn_value = sfn;
            send_mib(udp_sock, &mib, &ue_addr);
        }
    }
}