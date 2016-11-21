#include "iotgo_communicate_with_long_server.h"
#include "stdlib.h"


uint32_t updatatowebsocket(MySocketClient *p_mysocket)
{
	uint8_t buf[500] = { "GET /api/ws HTTP/1.1\r\n"
		"Host: iotgo.iteadstudio.com\r\n"
		"Connection: upgrade\r\n"
		"Upgrade: websocket\r\n"
		"Sec-WebSocket-Key: ITEADTmobiM0x1DabcEsnw==\r\n"
		"Sec-WebSocket-Version: 13\r\n\r\n" };

	uint8_t rece_buf[1500] = { 0 };
	uint32_t ret;

	ret = writeToServer(p_mysocket,buf,strnlen_s(buf,500));

	ret = readFromServer(p_mysocket, rece_buf, 1024);
	printf("%s", rece_buf);

	//system("pause");
	if (my_strstr(rece_buf, "HTTP/1.1 101 Switching Protocols") == 0)
	{
		printf("%s","Switching Protocols false\n");
		return FALSE;
	}

	printf("%s", "Switching Protocols ok\n");

	return TRUE;
}

uint32_t sendregistermessage(MySocketClient *p_mysocket)
{
	uint8_t model[10] = "PSF-B04-GL";
	uint8_t deviceID[10] = "10000294e0";
	uint8_t apikey[36] = "3ebb4c1f-5656-47b0-93c0-3c2ef573c50c";
	uint8_t rom_version[5] = "1.1.1";
	
	char send_buf[500] = {0};
	uint8_t rece_buf[1500] = {0};
	uint32_t len = 0;
	uint32_t ret = 0;

	cJSON *cjson_Websocket_send = NULL;
	char *cjson_send =  "{\
		\"userAgent\":\"device\",\"apikey\" : \"3ebb4c1f-5656-47b0-93c0-3c2ef573c50c\",\"deviceid\" : \"10000294e0\",\"action\" : \"register\",\"version\" : 2,\"romVersion\" : \"1.1.1\",\"model\" : \"PSF-B04-GL\",\"ts\" : 933}" ;


	if ((cjson_Websocket_send = cJSON_CreateObject()) != NULL)
	{
		cJSON_AddItemToObject(cjson_Websocket_send, "userAgent", cJSON_CreateString("device"));
		cJSON_AddItemToObject(cjson_Websocket_send, "apikey", cJSON_CreateString(apikey));
		cJSON_AddItemToObject(cjson_Websocket_send, "deviceid", cJSON_CreateString(deviceID));
		cJSON_AddItemToObject(cjson_Websocket_send, "action", cJSON_CreateString("register"));
		cJSON_AddItemToObject(cjson_Websocket_send, "version", cJSON_CreateNumber(2));
		cJSON_AddItemToObject(cjson_Websocket_send, "romVersion", cJSON_CreateString(rom_version));
		cJSON_AddItemToObject(cjson_Websocket_send, "model", cJSON_CreateString(model));
		cJSON_AddItemToObject(cjson_Websocket_send, "ts", cJSON_CreateNumber(933));
	}

	len = produceWebsocktData(cjson_send, send_buf);

	ret = writeToServer(p_mysocket,send_buf, len);
	ret = readFromServerNonBlock(p_mysocket, rece_buf, 1024);
	if (ret)
	{
		return TRUE;
	}
}

static int produceWebsocktData(char *data, char *buf)
{
	int data_len;
	int header_index = 0;
	int len = 0;
	int ret = -1;

	data_len = strlen(data);

	header_index = 0;
	buf[header_index++] = 0x81; /* Final package and text data type */
	if (data_len >= 0 && data_len <= 125)
	{
		buf[header_index++] = 0x80 + data_len;
	}
	else if (data_len >= 126 && data_len <= 65500)
	{
		buf[header_index++] = 0x80 + 126;
		buf[header_index++] = (data_len >> 8) & 0xFF;
		buf[header_index++] = (data_len) & 0xFF;
	}
	else
	{

	}

	buf[header_index++] = 0x00;   /* masking key = 0*/
	buf[header_index++] = 0x00;   /* masking key = 0*/
	buf[header_index++] = 0x00;   /* masking key = 0*/
	buf[header_index++] = 0x00;   /* masking key = 0*/
	
	memcpy(&buf[header_index], data, data_len);
	len = data_len + header_index;

	return len;
}

uint32_t heart_beat_thread(MySocketClient *p_mysocket)
{
	uint8_t heart_beat_data[6] = { 0x89,0x80,0x00,0x00,0x00,0x00 };
	uint8_t rece_buf[200] = {0};
	uint32_t ret;

	while (1)
	{
		/*send websocket Ping to Server*/		
		writeToServer(p_mysocket, heart_beat_data, 6);
		ret=readFromServerNonBlock(p_mysocket, rece_buf, 50);
		clear_buff(rece_buf, 50);
		if (ret)
		{
			ret = 0;			
			Sleep(5000);
		}
		else
		{
			return FALSE;
		}
	}
}