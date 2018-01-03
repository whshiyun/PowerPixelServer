/*
 *  drv.c
 *
 *  Created on: 2017年11月21日
 *  Author: gift
 */

#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>

//#include "MsgSubscribeServer/container/linkList.h"
#include "../../../../PowerServer/MsgSubscribeServer/container/linkList.h"
#include "system/systemIf.h"
#include "../inc/bsp.h"
#include "../inc/drv.h"

#define SAMPLING_INTERVAL_MS	(1000)

typedef struct __cycle_read_info {

	LIST_NODE node;

	void *file;
	DATA_CYCLE_READ_FUNC readHandle;
	bool isReadRawData;

}CYCLE_READ_INFO, *P_CYCLE_READ_INFO;

typedef void (*CYCLE_READ_FUNC)(size_t *buf, size_t count);

size_t cycleReadTask = (size_t)NULL;
volatile bool isCycleReadTaskEnable = false;
CYCLE_READ_INFO crInfoHead = {.node = {&crInfoHead.node, &crInfoHead.node}, .readHandle = NULL};


/*****************************************************************************
 * data process internal interface
 *****************************************************************************/
/*
 * 对深度数据进行必要的变换
 */
static void DataTransform(size_t *buf, size_t count) {
	printf("DataTransform \r\n");
}

/*
 * 对深度数据的处理算法
 */
static void DataProcAlgorithm(size_t *buf, size_t count) {
	printf("DataProcAlgorithm \r\n");
}

/*****************************************************************************
 * data process internal interface - end
 *****************************************************************************/

/*****************************************************************************
 * internal functions
 *****************************************************************************/
#ifndef IRQ_TRIGGER
static size_t RegisterCycleRead(CYCLE_READ_FUNC read) {

	isCycleReadTaskEnable = true;
	return (size_t)PPCreatTask(0, read, NULL);
}

static void UnRegisterCycleRead(size_t taskId) {

	isCycleReadTaskEnable = false;
}
#else
static off64_t RegisterCycleRead(CYCLE_READ_FUNC read) {

	return (size_t)0;
}

static void UnRegisterCycleRead(off64_t taskId) {

}
#endif

static size_t ReadCycleData(data_t *buf, size_t count, bool force) {

	printf("ReadCycleData \r\n");

	return 1;
}

static void ProcRegisterRead(bool isReadRawData, data_t *readBuf, unsigned int count) {

	P_CYCLE_READ_INFO pos = NULL;
	list_for_each_entry(pos, &crInfoHead.node, node) {
		if(isReadRawData == pos->isReadRawData) {
			if(NULL != pos->readHandle)
				pos->readHandle(pos->file, (unsigned char *)readBuf, count * (sizeof(data_t) / sizeof(unsigned char)));
		}
	}
}

static void CycleReadCallback() {

	static data_t readBuf[MAX_WIDTH_PIXEL_NUM * MAX_HEIGHT_PIXEL_NUM];
	static unsigned int count = 0;

	while (true) {

		PPDelay(SAMPLING_INTERVAL_MS);
		if(false == isCycleReadTaskEnable)
			break;

		count = ReadCycleData(readBuf, MAX_WIDTH_PIXEL_NUM * MAX_HEIGHT_PIXEL_NUM, false);
		if(0 == count)
			continue;

		ProcRegisterRead(true, readBuf, count);

		DataTransform(readBuf, count);
		DataProcAlgorithm(readBuf, count);

		ProcRegisterRead(false, readBuf, count);

		count = 0;
	}

	printf("CycleRead Exit \r\n");
}
/*****************************************************************************
 * internal functions - end
 *****************************************************************************/





/*****************************************************************************
 * operator interface
 *****************************************************************************/
int DrvOpen(void *file) {

	int ret = PP_OK;

	ret = BspOpen(file);

	return ret;
}

int DrvRelease(void *file) {

	int ret = PP_OK;

	ret = BspRelease(file);

	DrvUnRegCycleRead(file);

	return ret;
}

ssize_t DrvRead(void *file, char __user *buf, size_t count, loff_t *offset) {
	return PP_OK;
}

ssize_t DrvWrite(void *file, const char __user *buf, size_t count, loff_t *offset) {
	return PP_OK;
}

long DrvIoctl(void *file, unsigned int cmd, unsigned long arg) {

	switch(cmd) {
	case PP_READ_REG :
		printf("PP_READ_REG\r\n");
		break;
	case PP_WRITE_REG :
		printf("PP_WRITE_REG\r\n");
		break;
	default :
		break;
	}

	return PP_OK;
}

int DrvRegCycleRead(void *file, DATA_CYCLE_READ_FUNC read) {

	if(NULL == file)
		return PP_ERROR;

	P_CYCLE_READ_INFO pos = NULL;
	pos = list_first_entry(&crInfoHead.node, typeof(*pos), node);
	list_for_each_entry(pos, &crInfoHead.node, node) {
		if(file == pos->file) {
			return PP_OK;
		}
	}

	pos = (P_CYCLE_READ_INFO)malloc(sizeof(CYCLE_READ_INFO));
	pos->file = file;
	pos->readHandle = read;
	pos->isReadRawData = false;

	list_add(&crInfoHead.node, &pos->node);

	if((size_t)NULL == cycleReadTask) {
		cycleReadTask = RegisterCycleRead(CycleReadCallback);
	}

	return PP_OK;
}

int DrvUnRegCycleRead(void *file) {

	P_CYCLE_READ_INFO pos = NULL, n = NULL;
	list_for_each_entry_safe(pos, n, &crInfoHead.node, node) {
		if(file == pos->file) {
			list_del(&pos->node);
			free(pos);
			pos = n;
		}
	}

	if(true == list_empty(&crInfoHead.node)) {
		UnRegisterCycleRead(cycleReadTask);
		cycleReadTask = (size_t)NULL;
	}

	return PP_OK;
}

/*****************************************************************************
 * interface - end
 *****************************************************************************/

