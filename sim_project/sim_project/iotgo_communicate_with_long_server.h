#ifndef _IOTGO_COMMUNICATE_WITH_LONG_SERVER_H
#define _IOTGO_COMMUNICATE_WITH_LONG_SERVER_H

#include "mysocketclient.h"
#include "cJSON.h"

uint32_t updatatowebsocket(MySocketClient *p_mysocket);
uint32_t sendregistermessage(MySocketClient *p_mysocket);
static int produceWebsocktData(char *data, char *buf);
uint32_t heart_beat_thread(MySocketClient *p_mysocket);

#endif
