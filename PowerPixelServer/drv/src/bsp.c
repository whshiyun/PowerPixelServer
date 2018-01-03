/*
 * bsp.c
 *
 *  Created on: 2017Äê11ÔÂ21ÈÕ
 *      Author: gift
 */
#include "../inc/drvTypes.h"

//#include "bsp.h"

ssize_t BspRead(void *file, char __user *buf, size_t count, loff_t *offset) {
	return PP_OK;
}

ssize_t BspWrite(void *file, const char __user *buf, size_t count, loff_t *offset) {
	return PP_OK;
}

int BspOpen(void *file) {
	return PP_OK;
}

int BspRelease(void *file) {
	return PP_OK;
}

long BspIoctl(void *file, unsigned int cmd, unsigned long arg) {
	return PP_OK;
}

