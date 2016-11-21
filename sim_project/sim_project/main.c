#include "mysocketclient.h"
#include "main.h"
#include "uart.h"
#include "iotgo_communicate_with_long_server.h"

HANDLE hCom;

int main(void)
{	
	uint8_t rece_buf[2000] = {0};

	hCom = Serial_open(_T("COM7"),115200);
	MySocketClient *p = NULL;

	p = setupSocketClient("54.223.34.73", 8081);

#if 0
	ret = writeToServer(p,
"POST /dispatch/device HTTP/1.1\r\n\
Host: cn-disp.coolkit.cc\r\n\
Requestid: 2\r\n\
Content-Type: application/json\r\n\
Content-Length: 152\r\n\
\r\n\
{\"accept\":\"ws;2\",\"version\":2,\"ts\":933,\"deviceid\":\"100002cd82\",\"apikey\":\"adf20e49-3b57-4c39-b04a-f1f61e6fc5d3\",\"model\":\"BIT-EYE-GL\",\"romVersion\":\"1.1.1\"}\r\n", 281);
#endif
	
	

	while (1)
	{	
		if (cleanSocketClient(p) == IP_CLOSE_OK)
		{
			while (1)
			{
				if (connectToServer(p) == IP_CONNECT_OK)
				{
					if (updatatowebsocket(p) == TRUE)
					{
						if (sendregistermessage(p) == TRUE)
						{						
							if (heart_beat_thread(p) == FALSE)
							{
								break;
							}
						}
						else
						{
							break;
						}
					}
					else
					{
						break;
					}
				}
				else
				{
					break;
				}
			}
		}
	}

	Serial_close(hCom);

	return 0;
}
