#ifndef __MYSOCKETCLIENT_H__
#define __MYSOCKETCLIENT_H__

#include "stdint.h"
#include<windows.h>
#include "uart.h"


#define  IP_INITIAL				 0
#define  IP_START				 1
#define  IP_CONFIG				 2
#define  IP_IND					 3
#define  IP_GPRSACT				 4
#define  IP_STATUS 				 5	 
#define  IP_TCP_UDP_CONNECT		 6
#define  IP_CLOSE				 7
#define  IP_CONNECT_OK		     8

#define  CONNECT_ERROR			 9
#define  IP_CLOSE_ER			 10
#define  IP_CLOSE_OK			 11

#define  RESPOND_CONNECT         12
#define  RESPOND_WRITE           13
#define  RESPOND_READ            14
#define  RESPOND_CLOSE           15
#define  RESPOND_STATUS          16
#define	 RESPOND_HEART_PACK		 17

#define  RESPOND_OPMORE_CONNECT         18
#define  RESPOND_CLMORE_CONNECT         19


#define  RESPOND_OUT_TIME		 300
#define  START_UP_WAIT_TIME		 200

typedef struct mySocketClient_interface{
	uint8_t  server_addr[200];
	uint32_t server_port;
	uint32_t connect_status;
	uint32_t connect_channel;
}MySocketClient;

extern HANDLE hCom;

extern MySocketClient *setupSocketClient(uint8_t *server_ip,uint32_t server_port);
extern uint32_t cleanSocketClient(MySocketClient *pp_mysocket);
extern uint32_t connectToServer(MySocketClient *p_mysocket);
extern uint32_t readFromServer(MySocketClient *p_mysocket, uint8_t *buf, uint32_t len);
extern uint32_t readFromServerNonBlock(MySocketClient *p_mysocket, uint8_t *buf, uint32_t len);
extern uint32_t writeToServer(MySocketClient *p_mysocket,uint8_t *buf, uint32_t len);
extern uint32_t check_connect_status(MySocketClient *p_mysocket);
extern uint32_t open_more_ip_channel(void);
extern uint32_t close_more_ip_channel(void);
extern uint32_t setup_heart_packet(uint32_t second, void *send_content, void *rece_content);
extern BOOL open_heart_packet(MySocketClient *p_mysocket);
extern BOOL close_heart_packet(MySocketClient *p_mysocket);


uint32_t read_respond(uint8_t *rece_buf, uint32_t len, uint32_t type);
uint32_t start_up_wait_respond(uint8_t *rece_buf, uint32_t len);
void clear_buff(uint8_t *buf, int32_t len);
uint8_t *my_strstr(uint8_t *s1, uint8_t *s2);


#endif
