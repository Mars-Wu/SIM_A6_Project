#include "mysocketclient.h"

BOOL open_more_connect_flag = 0;


uint8_t *my_strstr(uint8_t *s1, uint8_t *s2)
{
	int n;
	if (*s2)                      //两种情况考虑  
	{
		while (*s1)
		{
			for (n = 0; *(s1 + n) == *(s2 + n); n++)
			{
				if (!*(s2 + n + 1))            //查找的下一个字符是否为'\0'  
				{
					return (uint8_t*)s1;
				}
			}
			s1++;
		}
		return NULL;
	}
	else
	{
		return (uint8_t*)s1;
	}
}

MySocketClient *setupSocketClient(uint8_t *server_ip,uint32_t server_port)
{
	MySocketClient *p_socket = NULL;
	
	p_socket = (MySocketClient *)malloc(sizeof(MySocketClient));

    if(!p_socket)
    {
		return NULL;
    }

    memset(p_socket,0,sizeof(MySocketClient));
    strcpy_s(p_socket->server_addr,20,server_ip);
    p_socket->server_port = server_port;
	p_socket->connect_status = IP_INITIAL;

    return p_socket;
}

uint32_t cleanSocketClient(MySocketClient *pp_mysocket)
{
	MySocketClient *p_mysocket = NULL;
	uint8_t command[200] = {"AT+CIPCLOSE"};
	uint8_t respond[200] = { 0 };
	uint32_t state;
	uint8_t c;

	p_mysocket = pp_mysocket;

	if (p_mysocket)
	{
		if (open_more_connect_flag)
		{
			strcat_s(command, 200,"=");
			switch (p_mysocket->connect_channel)
			{
				case 0:strcat_s(command, 200, "0"); break;
				case 1:strcat_s(command, 200, "1"); break;
				case 2:strcat_s(command, 200, "2"); break;
				case 3:strcat_s(command, 200, "3"); break;
			}
			
			strcat_s(command, 200,"\r\n");
			
			Serial_write(hCom,command,strnlen_s(command, 200));
		}
		else
		{
			if (check_connect_status(p_mysocket) != IP_CLOSE &&
				check_connect_status(p_mysocket) != IP_INITIAL)
			{
				strcat_s(command, 200, "\r\n");
				Serial_write(hCom, command, strnlen_s(command, 200));
			}
			else
			{
				return IP_CLOSE_OK;
			}
		}
		
		read_respond(respond, 200, RESPOND_CLOSE);
		if (strstr(respond, "OK"))
		{
			state = IP_CLOSE_OK;
		}
		else
		{
			state = IP_CLOSE_ER;
		}		
	}

	return state;
}

uint32_t connectToServer(MySocketClient *p_mysocket)
{
	uint8_t addr[100] = {0};
	uint8_t port[100] = {0};
	uint8_t command[200] = {"AT+CIPSTART=TCP,"};
	uint8_t respond[400] = {0};
	uint8_t respond1[600] = { 0 };
	uint8_t respond2[600] = { 0 };

	start_up_wait_respond(respond2, 600);

	strcpy_s(addr,20,p_mysocket->server_addr);
	_ltoa_s(p_mysocket->server_port,port,10,10);
	strcat_s(command, 200, addr);
	strcat_s(command, 200,",");
	strcat_s(command, 200,port);
	strcat_s(command, 200,"\r\n");

	Serial_write(hCom,command,strnlen_s(command, 200));

	read_respond(respond,400, RESPOND_CONNECT);

	if (open_more_connect_flag)
	{
		if (my_strstr(respond,"OK"))
		{
			if (my_strstr(respond, "+CIPNUM:0"))
			{
				printf("%s", "TCP:0 CONNECT OK\n");
				p_mysocket->connect_channel = 0;
				p_mysocket->connect_status = IP_CONNECT_OK;
				return IP_CONNECT_OK;
			}
			if (my_strstr(respond, "+CIPNUM:1"))
			{
				printf("%s", "TCP:1 CONNECT OK\n");
				p_mysocket->connect_channel = 1;
				p_mysocket->connect_status = IP_CONNECT_OK;
				return IP_CONNECT_OK;
			}
			if (my_strstr(respond, "+CIPNUM:2"))
			{
				printf("%s", "TCP:2 CONNECT OK\n");
				p_mysocket->connect_channel = 2;
				p_mysocket->connect_status = IP_CONNECT_OK;
				return IP_CONNECT_OK;
			}
			if (my_strstr(respond, "+CIPNUM:3"))
			{
				printf("%s", "TCP:3 CONNECT OK\n");
				p_mysocket->connect_channel = 3;
				p_mysocket->connect_status = IP_CONNECT_OK;
				return IP_CONNECT_OK;
			}
			
		}
		else
		{
			printf("%s", "TCP CONNECT NOK\n");
			return CONNECT_ERROR;
		}
	}
	else
	{
		if (my_strstr(respond, "OK") != 0)
		{
			printf("%s", "TCP CONNECT OK\n");
			p_mysocket->connect_channel = 0;
			p_mysocket->connect_status = IP_CONNECT_OK;

			return IP_CONNECT_OK;
		}
		else
		{
			printf("%s", "TCP CONNECT NOK\n");
			if (check_connect_status(p_mysocket) != IP_CLOSE)
			{
				Serial_write(hCom, "AT+CIPCLOSE\r\n", strnlen_s("AT+CIPCLOSE\r\n", 100));
				read_respond(respond1, 600, RESPOND_CLOSE);
			}
			return CONNECT_ERROR;
		}
	}
}

uint32_t readFromServer(MySocketClient *p_mysocket, uint8_t *buf, uint32_t len)
{
	uint8_t  rece_buf[1500] = { 0 };
	uint8_t  rece_buf1[1500] = { 0 };
	int32_t rece_cnt = 0;
	int32_t tmp=0,read_cnt, timeout = 0;

	if (p_mysocket->connect_status == IP_CONNECT_OK)
	{
		while (1)
		{
			if (read_cnt = Serial_read(hCom, rece_buf, 1024))
			{
				timeout = 0;			
				memcpy(buf+rece_cnt,rece_buf,read_cnt);	
				printf("%s", rece_buf);
				clear_buff(rece_buf,1024);
				rece_cnt += read_cnt;
				read_cnt = 0;
				tmp = 0;
			}
			else
			{
				timeout++;
				Sleep(1);
				if (timeout > RESPOND_OUT_TIME)
				{
					if (buf[0] == 0)
					{
						printf("%s", "read out time\n");
						return 0;
					}
					break;
				}
			}
		}

		return rece_cnt;
	}
}

uint32_t readFromServerNonBlock(MySocketClient *p_mysocket,uint8_t *buf, uint32_t len)
{
	uint32_t ret;

	ret = readFromServer(p_mysocket,buf,len);

	return ret;
}


uint32_t writeToServer(MySocketClient *p_mysocket, uint8_t* buf, uint32_t len)
{
	uint8_t command[200] = { "AT+CIPSEND=" };
	uint8_t data_len[100] = { 0 };
	uint8_t respond[500] = { 0 };
	uint8_t respond1[1024] = { 0 };
	uint32_t data_cnt = 0;

	if (p_mysocket->connect_status == IP_CONNECT_OK)
	{
		if (open_more_connect_flag)
		{
			command[11] = p_mysocket->connect_channel + '0';
			_ltoa_s(data_cnt, data_len, 10, 10);
			strcat_s(command, 200, ",");
			strcat_s(command, 200, data_len);
			strcat_s(command, 200, "\r\n");
		}
		else
		{
			_ltoa_s(len, data_len, 10, 10);
			strcat_s(command, 200, data_len);
			strcat_s(command, 200, "\r\n");
		}
		Serial_write(hCom, command, strnlen_s(command, 200));
		read_respond(respond, 50, RESPOND_WRITE);
		if (my_strstr(respond, "> "))
		{
			//Serial_write(hCom, buf, len);
			for (int i = 0; i < len; i++)
			{
				Serial_write(hCom, &buf[i], 1);
			}
			//system("pause");
		}
		else
		{
			return FALSE;
		}
	}

	return data_cnt;
}

uint32_t check_connect_status(MySocketClient *p_mysocket)
{
	uint8_t  command[200] = {0};
	uint8_t  status[100] = {"0,"};
	uint8_t  respond[600] = {0};
	uint8_t  respond1[600] = {0};
	uint32_t cnt=0;

	Serial_write(hCom,"AT+CIPSTATUS\r\n",strnlen_s("AT+CIPSTATUS\r\n",100));

	while (1)
	{
		if (Serial_read(hCom, respond1, 600))
		{
			cnt = 0;
			if (my_strstr(respond1, "CIPSTATUS:0,") != 0)
			{
				strcpy_s(respond, 600,respond1);
			}
			clear_buff(respond1, 600);
		}
		else
		{
			Sleep(1);
			cnt++;
			if (cnt > RESPOND_OUT_TIME)
			{
				break;
			}
		}
	}

	if (my_strstr(respond, "0,IP INITIAL") != 0)
	{
		printf("%s","IP_INITIAL\n");
		return IP_INITIAL;
	}
	if (my_strstr(respond, "0,IP START") != 0)
	{
		printf("%s", "IP_START");
		return IP_START;
	}
	if (my_strstr(respond, "0,IP CONFIG") != 0)
	{
		printf("%s", "IP_CONFIG\n");
		return IP_CONFIG;
	}
	if (my_strstr(respond, "0,IP IND") != 0)
	{
		printf("%s", "IP_IND\n");
		return IP_IND;
	}
	if (my_strstr(respond, "0,IP GPRSACT") != 0)
	{
		printf("%s", "IP_GPRSACT\n");
		return IP_GPRSACT;
	}
	if (my_strstr(respond, "0,IP STATUS") != 0)
	{
		printf("%s", "IP_STATUS\n");
		return IP_STATUS;
	}
	if (my_strstr(respond, "0,TCP/UDP CONNECTING") != 0)
	{
		printf("%s", "IP_TCP_UDP_CONNECT\n");
		return IP_TCP_UDP_CONNECT;
	}
	if (my_strstr(respond, "0,IP CLOSE") != 0)
	{
		printf("%s", "IP_CLOSE\n");
		return IP_CLOSE;
	}
	if (my_strstr(respond,"0,CONNECT OK") != 0)
	{
		printf("%s", "IP_CONNECT_OK\n");
		return IP_CONNECT_OK;
	}
}

uint32_t start_up_wait_respond(uint8_t *rece_buf, uint32_t len)
{
	uint32_t timeout = 0;
	uint32_t read_cnt = 0;
	uint32_t old_len = 0;
	uint32_t read_len = 0;
	uint8_t read_buf[600] = { 0 };


	while (1)
	{
		if (read_cnt = Serial_read(hCom, read_buf, len))
		{
			clear_buff(rece_buf, 600);
			timeout = 0;
			read_len += read_cnt;
			strcpy_s(rece_buf, len, read_buf);
			printf("%s", read_buf);
			clear_buff(read_buf, 600);
			if (my_strstr(rece_buf, "OK") != 0)
			{
				return read_len;
			}
		}
		else
		{
			Sleep(1);
			timeout++;
			if (timeout > START_UP_WAIT_TIME)
			{
				if (rece_buf[0] == 0)
				{
					printf("%s", "start up time out\n");
					return 0;
				}
				else
				{

					return read_len;
				}
			}
		}

	}
}


uint32_t read_respond(uint8_t *rece_buf, uint32_t len,uint32_t type)
{
	uint32_t timeout = 0;
	uint32_t read_cnt = 0;
	uint32_t addr_diff = 0;
	uint8_t read_buf[600] = { 0 };

	while (1)
	{
		if (read_cnt = Serial_read(hCom, read_buf, 600))
		{
			timeout = 0;
			strcat_s(rece_buf,len,read_buf);
			printf("%s", read_buf);
			if (type == RESPOND_CONNECT || type == RESPOND_CLOSE)
			{
				if (my_strstr(rece_buf, "OK\r\nOK"))
				{
					return;
				}
			}
			if (type == RESPOND_OPMORE_CONNECT || type == RESPOND_CLMORE_CONNECT)
			{
				if (my_strstr(read_buf, "OK"))
				{
					return;
				}
			}
			if (type == RESPOND_WRITE)
			{
				if (my_strstr(read_buf, "<"))
				{
					return;
				}
			}
			if (type == RESPOND_HEART_PACK)
			{
				if (my_strstr(read_buf, "OK"))
				{
					return;
				}
			}
			clear_buff(read_buf,512);
		}	
		else
		{
			timeout++;
			Sleep(1);
			if (timeout > RESPOND_OUT_TIME)
			{
				if (rece_buf[0] == 0)
				{
					printf("%s", "read out time\n");
					return 0;
				}
				return;
			}
		}
	}
}

uint32_t open_more_ip_channel(void)
{
	uint8_t command[200] = { "AT+CIPMUX=1\r\n" };
	uint8_t respond[200] = {0};

	Serial_write(hCom,command,strnlen_s(command,20));
	read_respond(respond, 200, RESPOND_OPMORE_CONNECT);
	if (my_strstr(respond, "OK"))
	{
		open_more_connect_flag = 1;
	}
	else
	{
		return FALSE;
	}

	return TRUE;
}

uint32_t close_more_ip_channel(void)
{
	uint8_t command[200] = { "AT+CIPMUX=0\r\n" };
	uint8_t respond[200] = { 0 };

	Serial_write(hCom, command, strnlen_s(command, 200) + 1);
	read_respond(respond, 200, RESPOND_CLMORE_CONNECT);
	if (my_strstr(respond, "OK"))
	{
		open_more_connect_flag = 0;
	}
	else
	{
		return FALSE;
	}

	return TRUE;
}

uint32_t setup_heart_packet(uint32_t second,void *send_content,void *rece_content)
{
	uint8_t command_s[200] = { "AT+CIPHCFG=0," };
	uint8_t command_t[200] = { "AT+CIPHCFG=1," };
	uint8_t command_r[200] = { "AT+CIPHCFG=2," };
	uint8_t second_len[200] = { 0 };
	uint8_t respond[200] = { 0 };

	_ltoa_s(second, second_len, 10, 10);
	strcat_s(command_s, 200, second_len);
	strcat_s(command_s, 200, "\r\n");
	Serial_write(hCom,command_s,strnlen_s(command_s, 200));
	read_respond(respond, 200, RESPOND_HEART_PACK);
	if (my_strstr(respond, "OK")==0)
	{
		printf("%s","set packet second false\n");
		return FALSE;
	}
	printf("%s", "set packet second ok\n");

	clear_buff(respond, 200);
	strcat_s(command_t, 200, send_content);
	strcat_s(command_t, 200, "\r\n");
	Serial_write(hCom, command_t, strnlen_s(command_t, 200));
	read_respond(respond, 200, RESPOND_HEART_PACK);
	if (my_strstr(respond, "OK") == 0)
	{
		printf("%s", "set packet send_content false\n");
		return FALSE;
	}
	printf("%s", "set packet send_content ok\n");

	clear_buff(respond, 200);
	strcat_s(command_r, 200, rece_content);
	strcat_s(command_r, 200, "\r\n");
	Serial_write(hCom, command_r, strnlen_s(command_r, 120));
	read_respond(respond, 200, RESPOND_HEART_PACK);
	if (my_strstr(respond, "OK") == 0)
	{
		printf("%s", "set packet rece_content false\n");
		return FALSE;
	}
	printf("%s", "set packet rece_content ok\n");

	return TRUE;

}

BOOL open_heart_packet(MySocketClient *p_mysocket)
{
	uint8_t command[200] = {"AT+CIPHMODE=1\r\n"};
	uint8_t respond[200] = {0};

	if (p_mysocket->connect_status == IP_CONNECT_OK)
	{
		Serial_write(hCom,command,strnlen_s(command,50));
		read_respond(respond, 50, RESPOND_HEART_PACK);
		if (my_strstr(respond, "OK") == 0)
		{
			printf("%s", "open heart_packe false\n");
			return FALSE;
		}
		printf("%s", "open heart_packe ok\n");
		return TRUE;
	}
}

BOOL close_heart_packet(MySocketClient *p_mysocket)
{
	uint8_t command[50] = { "AT+CIPHMODE=0\r\n" };
	uint8_t respond[50] = { 0 };

	if (p_mysocket->connect_status == IP_CONNECT_OK)
	{
		Serial_write(hCom, command, strnlen_s(command, 50));
		read_respond(respond, 50, RESPOND_HEART_PACK);
		if (my_strstr(respond, "OK") == 0)
		{
			printf("%s", "close heart_packe false\n");
			return FALSE;
		}
		printf("%s", "close heart_packe ok\n");
		return TRUE;
	}
}

void clear_buff(uint8_t *buf,int32_t len)
{
	int i = 0;

	while (len--)
	{
		buf[i] = 0;
		i++;
	}
}