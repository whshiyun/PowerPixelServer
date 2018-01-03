/*
 * server.c
 *
 *  Created on: 2017Äê11ÔÂ21ÈÕ
 *      Author: gift
 */

//#include <drvServer.h>
//#include <drvTypes.h>
//#include <sktServer.h>
#include <stddef.h>
#include <stdio.h>
#include <windows.h>

#include "../../../MsgSubscribeServer/msg/msgServer/msgCom.h"
#include "../../../MsgSubscribeServer/msg/msgServer/msgServer.h"
#include "../../drv/inc/drvTypes.h"
#include "../inc/drvServer.h"
#include "../inc/sktServer.h"

static P_MSG_SERVER pMsgServer = NULL;
static P_MSG_COM pMsgCom = NULL;

int StartServer() {

	pMsgServer = newMsgServer();
	if(NULL == pMsgServer)
		return PP_ERROR;

	pMsgCom = newMsgCom(pMsgServer);
	if(NULL == pMsgCom)
		return PP_ERROR;

	if(PP_OK != DrvServerStart(pMsgServer)) {
		return PP_ERROR;
	}

	if(PP_OK != StartSocketServer(pMsgCom)) {
		return PP_ERROR;
	}

	printf("PowerPixel server start success !!! \r\n");

	return PP_OK;
}

int StopServer() {

	CloseSocketServer(pMsgCom);
	DrvServerStop(pMsgServer);
	delMsgCom(pMsgCom);
	delMsgServer(pMsgServer);

	return 0;
}

