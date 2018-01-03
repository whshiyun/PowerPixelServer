/***********************************************
 *  /PowerPixelServer/server/src/drvServer.c
 *
 *  Created on: 2017年11月24日
 *  Author: gift
 *
 ***********************************************/
//#include <drv.h>
//#include <drvTypes.h>
//#include <serverTypes.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>

#include "../../../../PowerServer/MsgSubscribeServer/msg/msgDef.h"
#include "../../../../PowerServer/MsgSubscribeServer/msg/msgServer/msgCom.h"
#include "../../../../PowerServer/MsgSubscribeServer/msg/msgServer/msgServer.h"
#include "../../../../PowerServer/MsgSubscribeServer/msg/msgServer/msgServerDef.h"
#include "drv/inc/drv.h"
#include "drv/inc/drvTypes.h"
#include "../inc/serverTypes.h"
struct msg_bind_map {
	T_MSG_SN msgType;
	T_MSG_SN msgPriority;
	T_MSG_CALLBACK_FUNC func;
};

void static ListenDataCallback(void *file, unsigned char *buf, unsigned int count);

static bool MsgProcReadReg(T_MSG_SN msgType, u32 msgLen, u8 *pMsg, void *pProcParam, unsigned int recvPort);
static bool MsgProcWriteReg(T_MSG_SN msgType, unsigned char *pMsg, unsigned int msgLen, void *pProcParam, unsigned int recvPort);
static bool MsgProcSetFilter(T_MSG_SN msgType, unsigned char *pMsg, unsigned int msgLen, void *pProcParam, unsigned int recvPort);
static bool MsgProcBeginListenData(T_MSG_SN msgType, unsigned char *pMsg, unsigned int msgLen, void *pProcParam, unsigned int recvPort);
static bool MsgProcEndListenData(T_MSG_SN msgType, unsigned char *pMsg, unsigned int msgLen, void *pProcParam, unsigned int recvPort);
static bool MsgProcListenRawData(T_MSG_SN msgType, unsigned char *pMsg, unsigned int msgLen, void *pProcParam, unsigned int recvPort);
static bool MsgProcListenFilterData(T_MSG_SN msgType, unsigned char *pMsg, unsigned int msgLen, void *pProcParam, unsigned int recvPort);

struct msg_bind_map msgBindMapTab[] = {
		{SERVER_MSG_READ_REG, MSG_PRIORITY_READ_REG, (T_MSG_CALLBACK_FUNC)MsgProcReadReg},
		{SERVER_MSG_WRITE_REG, MSG_PRIORITY_WRITE_REG, (T_MSG_CALLBACK_FUNC)MsgProcWriteReg},
		{SERVER_MSG_SET_FILTER, MSG_PRIORITY_SET_FILTER, (T_MSG_CALLBACK_FUNC)MsgProcSetFilter},
		{SERVER_MSG_BEGIN_LISTEN_DATA, MSG_PRIORITY_BEGIN_LISTEN_DATA, (T_MSG_CALLBACK_FUNC)MsgProcBeginListenData},
		{SERVER_MSG_END_LISTEN_DATA, MSG_PRIORITY_END_LISTEN_DATA, (T_MSG_CALLBACK_FUNC)MsgProcEndListenData},
		{SERVER_MSG_LISTEN_RAW_DATA, MSG_PRIORITY_LISTEN_RAW_DATA, (T_MSG_CALLBACK_FUNC)MsgProcListenRawData},
		{SERVER_MSG_LISTEN_FILTER_DATA, MSG_PRIORITY_LISTEN_FILTER_DATA, (T_MSG_CALLBACK_FUNC)MsgProcListenFilterData},
};

static P_MSG_SERVER pMsgServer = NULL;
static int file;

static bool MsgProcReadReg(T_MSG_SN msgType, u32 msgLen, u8 *pMsg, void *pProcParam, unsigned int recvPort) {
	printf("MsgProcReadReg \r\n");

	unsigned int cmd = 0;
	reg_t reg = 0;
	DrvIoctl(&file, cmd, reg);

	//仅对该端口发送寄存器读取消息
	msgPushWithDes(pMsgServer, SERVER_MSG_REPORT_REG, (unsigned char *)&reg, sizeof(reg_t), recvPort);

	return false;
}

static bool MsgProcWriteReg(T_MSG_SN msgType, unsigned char *pMsg, unsigned int msgLen, void *pProcParam, unsigned int recvPort) {
	printf("MsgProcWriteReg \r\n");
	return false;
}

static bool MsgProcSetFilter(T_MSG_SN msgType, unsigned char *pMsg, unsigned int msgLen, void *pProcParam, unsigned int recvPort) {
	printf("MsgProcSetFilter \r\n");
	return false;
}

static bool MsgProcBeginListenData(T_MSG_SN msgType, unsigned char *pMsg, unsigned int msgLen, void *pProcParam, unsigned int recvPort) {
	printf("MsgProcBeginListenData \r\n");

//	P_MSG_COM_PORT_INFO recvPortInfo = (P_MSG_COM_PORT_INFO)recvPort;
//	msgSubscribeFull (pMsgServer, SERVER_MSG_REPORT_LISTEN_DATA, recvPortInfo->sendFunc,
//			recvPortInfo->pSendParam, MSG_PRIORITY_REPORT_LISTEN_DATA, false, true, recvPort);

	DrvRegCycleRead(&file, ListenDataCallback);

	return false;
}

static bool MsgProcEndListenData(T_MSG_SN msgType, unsigned char *pMsg, unsigned int msgLen, void *pProcParam, unsigned int recvPort) {
	printf("MsgProcEndListenData \r\n");

	DrvUnRegCycleRead(&file);

	return false;
}

static bool MsgProcListenRawData(T_MSG_SN msgType, unsigned char *pMsg, unsigned int msgLen, void *pProcParam, unsigned int recvPort) {
	printf("MsgProcListenRawData \r\n");
	return false;
}

static bool MsgProcListenFilterData(T_MSG_SN msgType, unsigned char *pMsg, unsigned int msgLen, void *pProcParam, unsigned int recvPort) {
	printf("MsgProcListenFilterData \r\n");
	return false;
}


//static bool MsgProcReportReg(T_MSG_SN msgType, unsigned char *pMsg, unsigned int msgLen) {
//	printf("MsgProcReportReg \r\n");
//	return false;
//}
//
//static bool MsgProcReportFilterConfig(T_MSG_SN msgType, unsigned char *pMsg, unsigned int msgLen) {
//	printf("MsgProcReportFilterConfig \r\n");
//	return false;
//}
//
//static bool MsgProcReportListenData(T_MSG_SN msgType, unsigned char *pMsg, unsigned int msgLen) {
//	printf("MsgProcReportListenData \r\n");
//	return false;
//}

void static ListenDataCallback(void *file, unsigned char *buf, unsigned int count) {

	printf("ListenDataCallback \r\n");
	printf("count = %d \r\n", count);

    msgPush(pMsgServer, SERVER_MSG_REPORT_LISTEN_DATA, buf,	count);
}

int DrvServerStart(P_MSG_SERVER pServer) {

	if(NULL == pServer)
		return PP_ERROR;

	if(NULL != pMsgServer)
		return PP_ERROR;

	pMsgServer = pServer;

	int ret = 0;

	if(PP_ERROR == DrvOpen(&file))
		return PP_ERROR;

	int n = ARRAY_SIZE(msgBindMapTab);
	for(int i = 0; i < n; i++) {
		if(false == msgSubscribe (pMsgServer, msgBindMapTab[i].msgType, msgBindMapTab[i].func,
					NULL, msgBindMapTab[i].msgPriority, false, true)) {
			ret ++;
		}
	}

	return ret;
}

int DrvServerStop(P_MSG_SERVER pServer) {

	if(pServer != pMsgServer)
		return PP_ERROR;

	int ret = 0;
	int n = ARRAY_SIZE(msgBindMapTab);
	for(int i = 0; i < n; i++) {
		if(false == msgUnsubscribe (pMsgServer,
				msgBindMapTab[i].msgType, msgBindMapTab[i].func, NULL)) {
			ret ++;
		}
	}

	if(PP_ERROR == DrvRelease(&file))
		return PP_ERROR;

	pMsgServer = NULL;

	return ret;
}
