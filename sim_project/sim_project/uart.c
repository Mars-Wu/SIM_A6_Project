 #include "uart.h"
#include "stdint.h"

extern HANDLE hCom;
/**
open serial
@param COMx: eg:_T("COM1")
@param BaudRate:
return 0 success ,return Negative is haed err
*/
HANDLE Serial_open(LPCWSTR COMx, int BaudRate)
{
	HANDLE hCom;
	COMMTIMEOUTS TimeOuts;
	DCB dcb = {0};
	hCom = CreateFile(
		COMx,
		GENERIC_READ | GENERIC_WRITE,
		0,
		0,
		OPEN_EXISTING,
		//FILE_ATTRIBUTE_NORMAL | FILE_FLAG_OVERLAPPED,
		0,//FILE_FLAG_OVERLAPPED,   //ͬ����ʽ �� �ص���ʽ   
		0
		);

	if (hCom == INVALID_HANDLE_VALUE)
	{
		DWORD dwError = GetLastError();
		printf("Sorry, failed to open the serial\n");
	}
	else
	{
		printf("The serial is successfully opened in a Baudrate %d!\n", BaudRate);
	}
		

	dcb.DCBlength = sizeof(DCB);

	if (!GetCommState(hCom, &dcb))
	{
		DWORD dwError = GetLastError();
		return(HANDLE)(-1);
	}

	dcb.BaudRate = BaudRate;   //������   
	dcb.ByteSize = 8;          //λ��   
	dcb.Parity = NOPARITY;     //��ż����   
	dcb.StopBits = ONESTOPBIT;  //ֹͣλ��   

	if (!SetCommState(hCom, &dcb))
	{
		DWORD dwError = GetLastError();
		return(HANDLE)(-1);
	}
	//if (!PurgeComm(hCom, PURGE_RXCLEAR))   return(HANDLE)(-1);

	SetupComm(hCom, 1024, 1024);

	//�趨����ʱ
	TimeOuts.ReadIntervalTimeout = MAXDWORD;
	TimeOuts.ReadTotalTimeoutMultiplier = 0;
	TimeOuts.ReadTotalTimeoutConstant = 0;
	SetCommTimeouts(hCom, &TimeOuts);

	PurgeComm(hCom, PURGE_RXCLEAR);
	PurgeComm(hCom, PURGE_TXCLEAR);
	return hCom;;
}



/**
serial read
@param Buf:data buf
@param size:
@return The len of read
*/
int Serial_read(HANDLE hCom, uint8_t *OutBuf, uint32_t size)
{
	uint32_t cnt = 0;

	ReadFile(hCom, OutBuf, size, &cnt, 0);
	
	return cnt;
	
}

/**
serial write
@param Buf:data buf
@param size:bytes of Buf
@return The len of writen
*/
int Serial_write(HANDLE hCom, void *Buf, int size)
{
	DWORD dw;
	WriteFile(hCom, Buf, size, &dw, NULL);
	return dw;
}

/**
serial close
*/
void Serial_close(HANDLE hCom)
{
	CloseHandle(hCom);
}
