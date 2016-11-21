#ifndef _UART_H
#define _UART_H


#include<TCHAR.H>   
#include<windows.h>
#include "stdio.h"
#include "stdint.h"

extern HANDLE hCom;

HANDLE Serial_open(LPCWSTR, int);
int Serial_read(HANDLE hCom, uint8_t *OutBuf, uint32_t size);
int Serial_write(HANDLE, const void*, int);
void Serial_close(HANDLE);
void clear_buf(unsigned char*, int);

#endif
