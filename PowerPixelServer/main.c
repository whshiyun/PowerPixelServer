/*
 * main.c
 *
 *  Created on: 2017Äê11ÔÂ21ÈÕ
 *      Author: gift
 */

#include <stdio.h>
#include <windows.h>

#include "server/inc/server.h"
//#include "server.h"
//#include "drv.h"

int main() {

//	ShowWindow(GetConsoleWindow(), SW_HIDE);

	StartServer();

#if 0
	char cmd;
	while(1) {
		Sleep(1);
		scanf("%c", &cmd);
		if('q' == cmd)
			break;
	}
#else
	while(1) {
	Sleep(3000);
	}
#endif
	StopServer();

	return 0;
}

