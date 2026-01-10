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
	ip_addr_t myIPaddr;
	IP_ADDR4(&myIPaddr, 192, 168, 0, 123);
	udp_bind(upcb, &myIPaddr, 8);


	/* configure destination IP address and port */
	ip_addr_t DestIPaddr;
	IP_ADDR4(&DestIPaddr, 192, 168, 0, 2);
	err= udp_connect(upcb, &DestIPaddr, 7);

	if (err == ERR_OK)
	{
		/* 2. Send message to server */
		// udpClient_send ();

		/* 3. Set a receive callback for the upcb */
		udp_recv(upcb, udp_receive_callback, NULL);
	}
}

void udpClient_send(void)
{
  struct pbuf *txBuf;
  char data[400];

  int len = sprintf(data, "{\"name\":\"DAQ1\",\"uptime\":40284,\"id\":1,\"data\":[{\"metric\":\"wheelSpeed\",\"time\":2039,\"unit\":\"RPM\",\"value\":42},{\"metric\":\"dynoLoad\",\"time\":2039,\"unit\":\"lbf\",\"value\":%f},{\"metric\":\"ambTemp\",\"time\":2039,\"unit\":\"C\",\"value\":420}]}", dataPacketNow.force);

  /* allocate pbuf from pool*/
  txBuf = pbuf_alloc(PBUF_TRANSPORT, len, PBUF_RAM);

  if (txBuf != NULL)
  {
    /* copy data to pbuf */
    pbuf_take(txBuf, data, len);

    /* send udp data */
    udp_send(upcb, txBuf);

    /* free pbuf */
    pbuf_free(txBuf);
  }
  HAL_GPIO_TogglePin(GPIOE, GPIO_PIN_14);
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


