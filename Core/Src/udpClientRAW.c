// Software originally by controllerstech.com, refactored and modified for DMA by Liam Homburger. GNU

#include "lwip/pbuf.h"
#include "lwip/udp.h"
#include "lwip/tcp.h"

#include "stdio.h"
#include "string.h"

#include <main.h>

#include "udpClientRAW.h"
#include "stm32f4xx_it.h"

#define CLAMP(x, min, max)  ((x) < (min) ? (min) : ((x) > (max) ? (max) : (x)))

void udp_receive_callback(void *arg, struct udp_pcb *upcb, struct pbuf *p, const ip_addr_t *addr, u16_t port);
//static void udpClient_send(void);

//struct dataPacket {
//    double force;
//    double RPM;
//    double temp;
//    double humidity;
//    double pressure;
//};

struct dataPacket dataPacketNow; // Current data packet
struct dataPacket dataPacketPrev; // Previous data, used to compare with dataPacketNow to only transmit when we have new data
extern struct valveData valveData;
extern struct PID_Data PID_Data;

struct udp_pcb *upcb;
char buffer[100];
int counter = 0;

int dataBatchingMax = 10; // Should be a define/constant, or query length of
int dataBatchingIndex = 0;

struct dataPacket dataArray[10];
struct valveData valveArray[10];
struct PID_Data PIDArray[10];

extern TIM_HandleTypeDef htim1;

ip_addr_t DestIPaddr;
ip_addr_t myIPaddr;

static char data[16384];

extern int isZeroing;
extern int isZeroingInit;

extern int rings;

//void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
//{
//	udpClient_send();
//}


/* IMPLEMENTATION FOR UDP CLIENT :   source:https://www.geeksforgeeks.org/udp-server-client-implementation-c/

1. Create UDP socket.
2. Send message to server.
3. Wait until response from server is received.
4. Process reply and go back to step 2, if necessary.
5. Close socket descriptor and exit.
*/


void udpClient_connect(void)
{
	err_t err;

	/* 1. Create a new UDP control block  */
	upcb = udp_new();

	/* Bind the block to module's IP and port */
	IP_ADDR4(&myIPaddr, 192, 168, 0, 123);
	udp_bind(upcb, &myIPaddr, 8);


	/* configure destination IP address and port */
	IP_ADDR4(&DestIPaddr, 192, 168, 0, 2);
	//err= udp_connect(upcb, &DestIPaddr, 7);
	udp_recv(upcb, udp_receive_callback, NULL);
//	if (err == ERR_OK)
//	{
//		/* 2. Send message to server */
//		// udpClient_send ();
//
//		/* 3. Set a receive callback for the upcb */
//		udp_recv(upcb, udp_receive_callback, NULL);
//	}
}

void udpClient_send(void)
{
//  HAL_GPIO_WritePin(GPIOD, GPIO_PIN_12, 0);

  struct pbuf *txBuf;
//  char data[16384];

  // Add term for acceleration
  // int len = sprintf(data, "{\"device\": \"DAQ1\",\"uptime\": 40284,\"id\": 1,\"headers\": [\"metric\", \"time\", \"unit\", \"value\"],\"data\": [[\"wheelSpeed\", 2039, \"RPM\", %f],[\"dynoLoad\", 2039, \"lbf\", %f],[\"ambTemp\", 2039, \"C\", %f],[\"ambPressure\", 2039, \"PSI\", 68],[\"ambHumidity\", 2039, \"RH\", 21],[\"outletTemp\", 2039, \"C\", %f],[\"tankTemp\", 2039, \"C\", 43],[\"alarm1\", 2039, \"bool\", 1],[\"alarm2\", 2039, \"bool\", 0],[\"valvePosition\", 2039, \"p\", %f],[\"loadThresh\", 2039, \"lbf\", 400],[\"eStop\", 2039, \"bool\", 0],[\"status\", 2039, \"errorCode\", %d]]}", dataPacketNow.RPM, dataPacketNow.force, dataPacketNow.temp, PID_Data.RPM_Target, (valveData.positionInSteps/valveData.pulsesPerRev)*100, valveData.targetPosition);
//
//  int len = 0;
//  len += sprintf(data + len, "{\"device\":\"DAQ1\",\"uptime\":%lu,\"id\":1,", HAL_GetTick());
//  len += sprintf(data + len, "\"headers\":[\"metric\",\"time\",\"unit\",\"value\"],");
//  for(int i = 0; i < dataBatchingMax; i++){
//	  len += sprintf(data + len, "\"data%d\":[", i);
//	  len += sprintf(data + len, "[\"wheelSpeed\",\"RPM\",%.2f],",    dataArray[i].RPM);
//	  len += sprintf(data + len, "[\"dynoLoad\",\"lbf\",%.2f],",      dataArray[i].force);
////	  len += sprintf(data + len, "[\"ambTemp\",\"C\",%.2f],",         dataArray[i].temp);
////	  len += sprintf(data + len, "[\"ambPressure\",\"PSI\",68],"      );
////	  len += sprintf(data + len, "[\"ambHumidity\",\"RH\",21],"       );
////	  len += sprintf(data + len, "[\"outletTemp\",\"C\",42],"         );
////	  len += sprintf(data + len, "[\"tankTemp\",\"C\",43],"           );
////	  len += sprintf(data + len, "[\"alarm1\",\"bool\",1],"           );
////	  len += sprintf(data + len, "[\"alarm2\",\"bool\",0],"           );
//	  len += sprintf(data + len, "[\"RPMTarget\",\"RPM\",%.2f],",     PIDArray[i].RPM_Target);
//	  len += sprintf(data + len, "[\"valvePosition\",\"p\",%.2f],",   (valveArray[i].positionInSteps / valveArray[i].pulsesPerRev) * 100.0f);
////	  len += sprintf(data + len, "[\"loadThresh\",\"lbf\",400],"      );
//	  len += sprintf(data + len, "[\"accel\",\"rpm/s\",%.2f]",       dataArray[i].acceleration);
////	  len += sprintf(data + len, "[\"eStop\",\"bool\",0],"            );
////	  len += sprintf(data + len, "[\"status\",\"errorCode\",%d]",     valveArray[i].targetPosition);
//	  if(i != dataBatchingMax - 1){
//		  len += sprintf(data + len, "],");
//	  }else{
//		  len += sprintf(data + len, "]");
//	  }
//  }
//  len += sprintf(data + len, "]}");

  int len = 0;
  len += snprintf(data + len, sizeof(data) - len, "{\"device\":\"DAQ1\",\"uptime\":%lu,\"id\":1,", HAL_GetTick());
  len += snprintf(data + len, sizeof(data) - len, "\"data\": {");
  len += snprintf(data + len, sizeof(data) - len, "\"headers\":[\"metric\",\"value\"],");
  len += snprintf(data + len, sizeof(data) - len, "\"tmp\": %.2f,", dataPacketNow.temp);
  len += snprintf(data + len, sizeof(data) - len, "\"prs\": %.2f,", dataPacketNow.pressure);
  len += snprintf(data + len, sizeof(data) - len, "\"hum\": %.2f,", dataPacketNow.humidity);
  len += snprintf(data + len, sizeof(data) - len, "\"freq\": 20,");
  len += snprintf(data + len, sizeof(data) - len, "\"cycles\": 10,");

  for(int i = 0; i < dataBatchingMax; i++){
      len += snprintf(data + len, sizeof(data) - len, "\"data%d\":[", i);
      len += snprintf(data + len, sizeof(data) - len, "[\"rSpd\",%.2f],",    dataArray[i].RPM);
      len += snprintf(data + len, sizeof(data) - len, "[\"dyLd\",%.6f],",      dataArray[i].force);
      len += snprintf(data + len, sizeof(data) - len, "[\"RPMT\",%.2f],",     PIDArray[i].RPM_Target);
      len += snprintf(data + len, sizeof(data) - len, "[\"vPos\",%.2f]",   (valveArray[i].positionInSteps / valveArray[i].pulsesPerRev) * 100.0f);
//      len += snprintf(data + len, sizeof(data) - len, "[\"acel\",%.2f]",        dataArray[i].acceleration);
//      len += snprintf(data + len, sizeof(data) - len, "[\"debug\",\"Test\",%d]", dataBatchingIndex);

      if(i < dataBatchingMax - 1){
          len += snprintf(data + len, sizeof(data) - len, "],"); // ← close data array, comma before next key
      } else {
          len += snprintf(data + len, sizeof(data) - len, "]");  // ← close last data array, no comma
      }
  }

  len += snprintf(data + len, sizeof(data) - len, "}}"); // ← close root object only, NO ]

  dataBatchingIndex = 0;

  /* allocate pbuf from pool*/
  txBuf = pbuf_alloc(PBUF_TRANSPORT, len, PBUF_RAM);

  if (txBuf != NULL)
  {
    /* copy data to pbuf */
    pbuf_take(txBuf, data, len);

    /* send udp data */
//    udp_send(upcb, txBuf);
    udp_sendto(upcb, txBuf, &DestIPaddr, 7);

    /* free pbuf */
    pbuf_free(txBuf);
  }
//  HAL_GPIO_TogglePin(GPIOE, GPIO_PIN_14);
//  HAL_GPIO_WritePin(GPIOD, GPIO_PIN_12, 1);
}

void udp_receive_callback(void *arg, struct udp_pcb *upcb, struct pbuf *p, const ip_addr_t *addr, u16_t port)
{

	char CWBuffer[6];
	char subCWBuffer[4];

    // Make sure buffer is large enough and null-terminated
    if (p->len >= sizeof(buffer))
        p->len = sizeof(buffer) - 1;

    strncpy(buffer, (char *)p->payload, p->len);
    buffer[p->len] = '\0'; // null-terminate the string

    // Increment message count
    counter++;

    memcpy(CWBuffer, buffer, 5);
    CWBuffer[5] = '\0';

    memcpy(subCWBuffer, &buffer[6], 3);
    subCWBuffer[3] = '\0';

    char *val = strrchr(buffer, ',');
    double num = atof(val + 1);

    // Check if the message is "Hello"
    if (strcmp(CWBuffer, "VALVE") == 0) {
        if(strcmp(subCWBuffer, "POS") == 0){
        	num = CLAMP(num, 0, 100);
        	int stepPosition = (num/100)*valveData.pulsesPerRev;
			valveData.targetPosition = stepPosition;
        }else if(strcmp(subCWBuffer, "ZER") == 0){
		    isZeroing = 1;
		    isZeroingInit = 1;
        }else if(strcmp(subCWBuffer, "PPR") == 0){
        	valveData.pulsesPerRev = num;
        }else if(strcmp(subCWBuffer, "GRO") == 0){
        	valveData.gearReduction = num;
        }
    }else if(strcmp(CWBuffer, "COPID") == 0){
    	if(strcmp(subCWBuffer, "RPM") == 0){
    		PID_Data.RPM_Target = (float) num;
    	}else if(strcmp(subCWBuffer, "ACU") == 0){
    		PID_Data.accum = (float) num;
    	}
    }else if(strcmp(CWBuffer, "ENPID") == 0){
    	if(strcmp(subCWBuffer, "RPM") == 0){
    		PID_Data.RPM_EN = (int) num;
    	}
    }else if(strcmp(CWBuffer, "FRAMP") == 0){
    	if(strcmp(subCWBuffer, "RPM") == 0){
    		PID_Data.RPM_End_Target = (float) num;
    	}else if(strcmp(subCWBuffer, "RTE") == 0){
    		PID_Data.RPM_Ramp_Rate = (float) num;
    	}else if(strcmp(subCWBuffer, "ENA") == 0){
    		PID_Data.RPM_RAMP_EN = (int) num;
    	}else if(strcmp(subCWBuffer, "IMX") == 0){
    		PID_Data.accumMax = (float) num;
    	}else if(strcmp(subCWBuffer, "KPT") == 0){
    		PID_Data.KP = (float) num;
    	}else if(strcmp(subCWBuffer, "KIT") == 0){
    		PID_Data.KI = (float) num;
    	}else if(strcmp(subCWBuffer, "KDT") == 0){
    		PID_Data.KD = (float) num;
    	}
    }else if(strcmp(CWBuffer, "BELLR") == 0){
    	if(strcmp(subCWBuffer, "RNG") == 0){
    		rings = (int) num;
    	}
    }
    // Free the receive pbuf
    pbuf_free(p);
}


