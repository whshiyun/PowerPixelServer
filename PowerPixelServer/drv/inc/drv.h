/*
 * drv.h
 *
 *  Created on: 2017Äê11ÔÂ21ÈÕ
 *      Author: gift
 */

#ifndef INC_DRV_DRV_H_
#define INC_DRV_DRV_H_

#include "drvTypes.h"

#define MAX_WIDTH_PIXEL_NUM			(320)
#define MAX_HEIGHT_PIXEL_NUM		(240)

typedef void (*DATA_CYCLE_READ_FUNC)(void *file, unsigned char *buf, unsigned int count);

typedef struct __pp_msg {
	size_t addr;
	size_t len;
	size_t *buf;
}PP_MSG, *P_PP_MSG;

#define PP_READ_REG			(0x00)
#define PP_WRITE_REG		(0x01)

ssize_t DrvRead(void *file, char __user *buf, size_t count, loff_t *offset);
ssize_t DrvWrite(void *file, const char __user *buf, size_t count, loff_t *offset);
int DrvOpen(void *file);
int DrvRelease(void *file);
long DrvIoctl(void *file, unsigned int cmd, unsigned long arg);

int DrvRegCycleRead(void *file, DATA_CYCLE_READ_FUNC read);
int DrvUnRegCycleRead(void *file);

#endif /* INC_DRV_DRV_H_ */
