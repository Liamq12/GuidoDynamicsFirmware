// Software originally by controllerstech.com, refactored and modified for DMA by Liam Homburger. GNU

#include "lwip/pbuf.h"
#include "lwip/udp.h"
#include "lwip/tcp.h"

#include "stdio.h"
#include "string.h"

#include <main.h>

#include "udpClientRAW.h"


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

struct udp_pcb *upcb;
char buffer[100];
int counter = 0;

extern TIM_HandleTypeDef htim1;

ip_addr_t DestIPaddr;
ip_addr_t myIPaddr;


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
  HAL_GPIO_WritePin(GPIOE, GPIO_PIN_14, 0);

  struct pbuf *txBuf;
  char data[800];

  int len = sprintf(data, "{\"device\": \"DAQ1\",\"uptime\": 40284,\"id\": 1,\"headers\": [\"metric\", \"time\", \"unit\", \"value\"],\"data\": [[\"wheelSpeed\", 2039, \"RPM\", %f],[\"dynoLoad\", 2039, \"lbf\", %f],[\"ambTemp\", 2039, \"C\", 67],[\"ambPressure\", 2039, \"PSI\", 68],[\"ambHumidity\", 2039, \"RH\", 21],[\"outletTemp\", 2039, \"C\", 42],[\"tankTemp\", 2039, \"C\", 43],[\"alarm1\", 2039, \"bool\", 1],[\"alarm2\", 2039, \"bool\", 0],[\"valvePosition\", 2039, \"p\", 9],[\"loadThresh\", 2039, \"lbf\", 400],[\"eStop\", 2039, \"bool\", 0],[\"status\", 2039, \"errorCode\", 0]]}", dataPacketNow.RPM, dataPacketNow.force);

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
  HAL_GPIO_WritePin(GPIOE, GPIO_PIN_14, 1);
}


void udp_receive_callback(void *arg, struct udp_pcb *upcb, struct pbuf *p, const ip_addr_t *addr, u16_t port)
{
	/* Copy the data from the pbuf */
	strncpy (buffer, (char *)p->payload, p->len);

	/*increment message count */
	counter++;

	/* Free receive pbuf */
	pbuf_free(p);


}


