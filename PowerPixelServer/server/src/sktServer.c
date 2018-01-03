
/***********************************************
 *  /PowerPixelServer/server/src/sktServer.c
 *
 *  Created on: 2017��11��28��
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

//socket�ķ��Ͳ����ṹ
typedef struct __socket_send_param {
	MSG_COM_PORT_INFO port;//��socket�˿���ͨ��ģ���еĶ˿���Ϣ
	SOCKET serSocket;//socket
	struct in_addr remoteIp;//��socketͨ�ŽӿڶԶ˵ĵ�ַ
	u_short remotePort;//��socketͨ�ŽӿڶԶ˵Ķ˿ڵ�ַ
	//struct sockaddr_in remoteAddr;
}SOCKET_SEND_PARAM, *P_SOCKET_SEND_PARAM;

//socket����������������������socket����Ϣ
typedef struct __socket_server_start_param {
	P_MSG_COM pCom;//��ýӿ����ӵ�ͨ��ģ����ڵ�ַ
	struct in_addr recvIp;//socket�������ı��ص�ַ
	u_short recvPort;//socket�������ı���IP
}SOCKET_SERVER_START_PARAM, *P_SOCKET_SERVER_START_PARAM;

HANDLE socketServerProcHandle = 0;//socket�������Ľ��̾����Ŀǰֻ֧��1��socket������
bool socketServerDestroy = false;//socket��������Ч��־�������رշ��������̵ı���
SOCKET_SERVER_START_PARAM startParam = { 0 };//������������������Ŀǰֻ֧��1��socket������

/*******************************************************************
 * socketģ���ṩ��msg server��ͨ��ģ��ķ��ͻص�����
 * ��msg server��ͨ��ģ����Ҫ�˿����ⷢ����Ϣʱ�����øú���
 * �ú����ڶ˿ڰ�ʱ�ṩ��msg server��ͨ��ģ��
 *******************************************************************/
static bool SocketSendMsg(T_MSG_SN msgType,	unsigned char *pMsg,
		unsigned int msgLen, P_SOCKET_SEND_PARAM pParam) {

	char buf[msgLen + sizeof(SOCKET_MSG_HEAD)];
	int headLend = mkSocketHead(buf, msgType, msgLen, TIMER_TIME_OUT_NEVER);
	if(0 > headLend)
		return false;

	memcpy(buf + headLend, pMsg, msgLen);

	printf("send to ip ��%s , port : %u , msgLen : %u\r\n", inet_ntoa(pParam->remoteIp), pParam->remotePort, msgLen);
	printf("SocketSendMsg : type : 0x%x \r\n", (char)buf[0]);

	if(SOCKET_ERROR == serverSendUDP(pParam->serSocket, pParam->remoteIp, pParam->remotePort, buf, sizeof(buf))) {
		char str[10] = { 0 };
		itoa(WSAGetLastError(),str,10);
		printf("SocketSend error code : "); printf(str); printf("\r\n");
	}

	//������ʱ��ΪֻҪmsg server����Ϣ���ⷢ�ͣ��ö˿ھ�һֱ���ڼ���״̬
	//������ʵ�ǲ�̫����ģ�Ӧ��ͨ���˿ڰ�ʱָ����ȷ��ʱ�䣬����Ѿ�����ͨ��socket��Ϣ��ʽʵ����
	//���ڽ�ȥ���˴��ĵ���
	keepPortActive(&pParam->port);

	return false;
}

/*******************************************************************
 * socket�������ķ�����
 * �ú���Ϊsocket��������ʵ��ִ�к�����һ��Ӧ�ö����߳�����
 * �ú������𴴽�socket����ˣ���ʱΪUDPͨ�ţ���������socket���ݽ���
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

	//�ݶ�Ŀǰͬʱ�������4��MAX_CONNECT_PORTS_NUM������ͬ�ķ���
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
    	//ͨ��UDP�������ݣ�ÿ���������DEFAULT_ONCE_RECV_CONINUE_TIMEʱ��
    	recvLen = serverRecvUDP(serSocket, recvBuf, sizeof(recvBuf),
    			&recvIp, &recvPort, DEFAULT_ONCE_RECV_CONINUE_TIME);

    	//���յ�����С����Ч�ֽڳ��ȣ�����д���������Ϊû�н��յ����ݣ�ת���ٴν���
        if (recvLen >= (int)sizeof(SOCKET_MSG_HEAD))
        {
        	P_SOCKET_MSG_HEAD pHead = (P_SOCKET_MSG_HEAD)recvBuf;
        	printf("���ܵ�һ�����ӣ�%s , port : %u, len = %d \r\n", inet_ntoa(recvIp), recvPort, recvLen);
            printf("msg type : %d, msg len : %d \r\n", pHead->msgType, pHead->msgLen);

            //��ѯ��ǰ�˿��Ƿ��Ѿ��󶨣������ˣ�����ö˿ڵ���msgComPortRecvCallback
            //surplus��¼һ����ǰδ��ʹ�ù���sendParams��ţ�����Ҫ�½��˿ڰ󶨣���ʹ��
            //��δ��ʹ�õ�sendParams�����ж˿ڰ�
            int i = 0, surplus = MAX_CONNECT_PORTS_NUM;
            for(i = 0; i < MAX_CONNECT_PORTS_NUM; i++) {
            	if(false == sendParams[i].port.enable) {
            		surplus = i;
            		continue;
            	}
            	if((recvIp.S_un.S_addr == sendParams[i].remoteIp.S_un.S_addr) && (recvPort == sendParams[i].remotePort)) {
            		//���ö˿��Ѿ��󶨹��������ͨ��ģ��Ľ��ջص�����������������ѭ��
            		msgComPortRecvCallback(pHead->msgType, (unsigned char *)(pHead + 1),
            				pHead->msgLen, &sendParams[i].port, NULL);
            		break;
            	}
            }

            //���˿ڲ����ڣ�����pCom bind�˶˿ڣ�bind�����msgComPortRecvCallback
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
 * ����һ��socket�����
 * �ú�����Ϊsocket����˴��������߳�
 *******************************************************************/
int StartSocketServer(P_MSG_COM pMsgCom) {

	//����socket����������������ݶ�Ϊ���²�����������ͨ���������ݷ�ʽ��Ϊ�ɱ����������
	startParam.pCom = pMsgCom;
	startParam.recvIp.S_un.S_addr = INADDR_ANY;
	startParam.recvPort = atoi((const char *)SOCKET_DEFAULT_SERVER_PORT_NUM);

	//����socket������߳�
	socketServerDestroy = false;
    if(NULL != (socketServerProcHandle = PPCreatTask((MAX_RECV_LENTH * 8), SocketServerProc, &startParam)))
    	return PP_OK;

    return PP_ERROR;
}

/*******************************************************************
 * �ر�һ��socket�����
 *******************************************************************/
int CloseSocketServer(P_MSG_COM pMsgCom) {

	if(NULL == socketServerProcHandle)
		return 0;

	//socketServerDestroy��λture����socket������̻߳ص�����������ѭ�����˳��������������߳�
	//�÷�ʽΪ�ȫ���߳̽�����ʽ
	socketServerDestroy = true;

	//�ȴ�socketServerProcHandle���߳̽��������ȴ���ʱδDEFAULT_ONCE_RECV_CONINUE_TIME������
	//���������Ľ�������ʱ��
	if(WAIT_OBJECT_0 != WaitForSingleObject(socketServerProcHandle, 2 * DEFAULT_ONCE_RECV_CONINUE_TIME)) {
		printf("Close Socket fail !!!\r\n");
		return PP_ERROR;
	}

	return PP_OK;
}



