
/***********************************************
 *  /PowerPixelServer/server/src/sktServer.c
 *
 *  Created on: 2017年11月28日
 *  Author: gift
 *
 ***********************************************/

#include <stdbool.h>
#include <stdio.h>
#include <winsock2.h>

#include "../../../../PowerServer/MsgSubscribeServer/msg/msgServer/msgCom.h"
#include "../../../../PowerServer/MsgSubscribeServer/msg/msgServer/msgServerDef.h"
#include "../../../../PowerServer/MsgSubscribeServer/msg/msgServer/msgTimeDef.h"
#include "../../../../PowerSocket/socket/typedef.h"
#include "../../../../PowerSocket/socket/socketMsg.h"
#include "../../../../PowerSocket/socket/socketServer.h"
#include "drv/inc/drvTypes.h"
#include "system/systemIf.h"


#define MAX_RECV_LENTH						((320 * 240 * 4) + 64)
#define MAX_CONNECT_PORTS_NUM				(4)
#define DEFAULT_ONCE_RECV_CONINUE_TIME		(100)//ms

//socket的发送参数结构
typedef struct __socket_send_param {
	MSG_COM_PORT_INFO port;//该socket端口在通信模块中的端口信息
	SOCKET serSocket;//socket
	struct in_addr remoteIp;//该socket通信接口对端的地址
	u_short remotePort;//该socket通信接口对端的端口地址
	//struct sockaddr_in remoteAddr;
}SOCKET_SEND_PARAM, *P_SOCKET_SEND_PARAM;

//socket服务启动参数，用来创建socket的信息
typedef struct __socket_server_start_param {
	P_MSG_COM pCom;//与该接口连接的通信模块入口地址
	struct in_addr recvIp;//socket服务器的本地地址
	u_short recvPort;//socket服务器的本地IP
}SOCKET_SERVER_START_PARAM, *P_SOCKET_SERVER_START_PARAM;

HANDLE socketServerProcHandle = 0;//socket服务器的进程句柄，目前只支持1个socket服务器
bool socketServerDestroy = false;//socket服务器有效标志，用来关闭服务器进程的变量
SOCKET_SERVER_START_PARAM startParam = { 0 };//服务器的启动参数，目前只支持1个socket服务器

/*******************************************************************
 * socket模块提供给msg server的通信模块的发送回调函数
 * 当msg server的通信模块需要端口向外发送消息时，调用该函数
 * 该函数在端口绑定时提供给msg server的通信模块
 *******************************************************************/
static bool SocketSendMsg(T_MSG_SN msgType,	unsigned char *pMsg,
		unsigned int msgLen, P_SOCKET_SEND_PARAM pParam) {

	char buf[msgLen + sizeof(SOCKET_MSG_HEAD)];
	int headLend = mkSocketHead(buf, msgType, msgLen, TIMER_TIME_OUT_NEVER);
	if(0 > headLend)
		return false;

	memcpy(buf + headLend, pMsg, msgLen);

	printf("send to ip ：%s , port : %u , msgLen : %u\r\n", inet_ntoa(pParam->remoteIp), pParam->remotePort, msgLen);
	printf("SocketSendMsg : type : 0x%x \r\n", (char)buf[0]);

	if(SOCKET_ERROR == serverSendUDP(pParam->serSocket, pParam->remoteIp, pParam->remotePort, buf, sizeof(buf))) {
		char str[10] = { 0 };
		itoa(WSAGetLastError(),str,10);
		printf("SocketSend error code : "); printf(str); printf("\r\n");
	}

	//这里暂时认为只要msg server有消息向外发送，该端口就一直处于激活状态
	//这里其实是不太合理的，应该通过端口绑定时指定正确的时间，这点已经可以通关socket消息格式实现了
	//后期将去掉此处的调用
	keepPortActive(&pParam->port);

	return false;
}

/*******************************************************************
 * socket服务器的服务函数
 * 该函数为socket服务器的实际执行函数，一般应该独立线程运行
 * 该函数负责创建socket服务端（暂时为UDP通信），并监听socket数据接收
 *******************************************************************/
static void SocketServerProc(P_SOCKET_SERVER_START_PARAM pSp) {

	if(NULL == pSp)
		return ;

	if(NULL == pSp->pCom)
		return ;

	if(NULL == pSp->pCom->pServer)
		return ;

	SOCKET serSocket = startUDPSocketServer(pSp->recvIp, pSp->recvPort);
	if(INVALID_SOCKET == serSocket)
		return ;

	//暂定目前同时最多链接4（MAX_CONNECT_PORTS_NUM）个不同的服务
	SOCKET_SEND_PARAM sendParams[MAX_CONNECT_PORTS_NUM];
	for(int i = 0; i<MAX_CONNECT_PORTS_NUM; i++) {
		msgComInitPort(&sendParams[i].port);
	}

	struct in_addr recvIp;
	u_short recvPort = 0;
	char recvBuf[MAX_RECV_LENTH];
	int recvLen;
    while (!socketServerDestroy)
    {
    	//通过UDP接收数据，每次最大阻塞DEFAULT_ONCE_RECV_CONINUE_TIME时长
    	recvLen = serverRecvUDP(serSocket, recvBuf, sizeof(recvBuf),
    			&recvIp, &recvPort, DEFAULT_ONCE_RECV_CONINUE_TIME);

    	//若收到了最小的有效字节长度，则进行处理，否则认为没有接收到数据，转入再次接收
        if (recvLen >= (int)sizeof(SOCKET_MSG_HEAD))
        {
        	P_SOCKET_MSG_HEAD pHead = (P_SOCKET_MSG_HEAD)recvBuf;
        	printf("接受到一个连接：%s , port : %u, len = %d \r\n", inet_ntoa(recvIp), recvPort, recvLen);
            printf("msg type : %d, msg len : %d \r\n", pHead->msgType, pHead->msgLen);

            //查询当前端口是否已经绑定，若绑定了，则向该端口调用msgComPortRecvCallback
            //surplus记录一个当前未被使用过的sendParams序号，若需要新建端口绑定，则使用
            //该未被使用的sendParams来进行端口绑定
            int i = 0, surplus = MAX_CONNECT_PORTS_NUM;
            for(i = 0; i < MAX_CONNECT_PORTS_NUM; i++) {
            	if(false == sendParams[i].port.enable) {
            		surplus = i;
            		continue;
            	}
            	if((recvIp.S_un.S_addr == sendParams[i].remoteIp.S_un.S_addr) && (recvPort == sendParams[i].remotePort)) {
            		//若该端口已经绑定过，则调用通信模块的接收回调函数，并跳出搜索循环
            		msgComPortRecvCallback(pHead->msgType, (unsigned char *)(pHead + 1),
            				pHead->msgLen, &sendParams[i].port, NULL);
            		break;
            	}
            }

            //若端口不存在，则向pCom bind此端口，bind后调用msgComPortRecvCallback
            if((MAX_CONNECT_PORTS_NUM == i) && (MAX_CONNECT_PORTS_NUM != surplus)) {
            	sendParams[surplus].serSocket = serSocket;
            	sendParams[surplus].remoteIp = recvIp;
            	sendParams[surplus].remotePort = recvPort;

            	bindMsgComPort(pSp->pCom, &sendParams[surplus].port,
            			(T_MSG_CALLBACK_FUNC)SocketSendMsg, &sendParams[surplus], pHead->timeout);

        		msgComPortRecvCallback(pHead->msgType, (unsigned char *)(pHead + 1),
        				pHead->msgLen,  &sendParams[surplus].port, NULL);
            }
        } else if(SOCKET_ERROR == recvLen){
        	char str[10] = { 0 };
        	itoa(WSAGetLastError(),str,10);
        	printf("SocketRecv error code : "); printf(str); printf("\r\n");
        }
    }

    closeUDPSocketServer(serSocket);
    socketServerDestroy = false;
}

/*******************************************************************
 * 启动一个socket服务端
 * 该函数将为socket服务端创建独立线程
 *******************************************************************/
int StartSocketServer(P_MSG_COM pMsgCom) {

	//设置socket服务端启动参数，暂定为如下参数，今后可以通过参数传递方式变为可变更启动参数
	startParam.pCom = pMsgCom;
	startParam.recvIp.S_un.S_addr = INADDR_ANY;
	startParam.recvPort = atoi((const char *)SOCKET_DEFAULT_SERVER_PORT_NUM);

	//创建socket服务端线程
	socketServerDestroy = false;
    if(NULL != (socketServerProcHandle = PPCreatTask((MAX_RECV_LENTH * 8), SocketServerProc, &startParam)))
    	return PP_OK;

    return PP_ERROR;
}

/*******************************************************************
 * 关闭一个socket服务端
 *******************************************************************/
int CloseSocketServer(P_MSG_COM pMsgCom) {

	if(NULL == socketServerProcHandle)
		return 0;

	//socketServerDestroy置位ture，则socket服务端线程回调函数会跳出循环，退出函数，并结束线程
	//该方式为最安全的线程结束方式
	socketServerDestroy = true;

	//等待socketServerProcHandle该线程结束，最大等待超时未DEFAULT_ONCE_RECV_CONINUE_TIME的两倍
	//即：两倍的接收阻塞时间
	if(WAIT_OBJECT_0 != WaitForSingleObject(socketServerProcHandle, 2 * DEFAULT_ONCE_RECV_CONINUE_TIME)) {
		printf("Close Socket fail !!!\r\n");
		return PP_ERROR;
	}

	return PP_OK;
}



