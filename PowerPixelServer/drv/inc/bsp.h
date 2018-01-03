/*
 * bsp.h
 *
 *  Created on: 2017Äê11ÔÂ21ÈÕ
 *      Author: gift
 */

#ifndef INC_BSP_H_
#define INC_BSP_H_

#include "drvTypes.h"

	ssize_t BspRead(void *file, char __user *buf, size_t count, loff_t *offset);
	ssize_t BspWrite(void *file, const char __user *buf, size_t count, loff_t *offset);
	int BspOpen(void *file);
	int BspRelease(void *file);
	long BspIoctl(void *file, unsigned int cmd, unsigned long arg);

#endif /* INC_BSP_H_ */
