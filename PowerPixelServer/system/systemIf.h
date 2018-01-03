/***********************************************
 *  /PowerPixelServer/system/systemIF.h
 *
 *  Created on: 2017Äê11ÔÂ22ÈÕ
 *  Author: gift
 *
 ***********************************************/

#ifndef SYSTEM_SYSTEMIF_H_
#define SYSTEM_SYSTEMIF_H_

#define windows

#ifdef windows

#include <windows.h>

#define PPDelay(ms) 			Sleep((ms))
#define PPCreatTask(size, callback, args) 	CreateThread( \
												 NULL, \
												 size, \
												 (LPTHREAD_START_ROUTINE)(callback), \
												 args, \
												 0, \
												 NULL) \

#endif

#endif /* SYSTEM_SYSTEMIF_H_ */
