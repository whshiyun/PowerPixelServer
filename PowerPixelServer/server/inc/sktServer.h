/***********************************************
 *  /PowerPixelServer/server/inc/sktServer.h
 *
 *  Created on: 2017��11��28��
 *  Author: gift
 *
 ***********************************************/

#ifndef SERVER_INC_SOCKETSERVER_H_
#define SERVER_INC_SOCKETSERVER_H_


#define SOCKET_MSG_READ_REG					SERVER_MSG_READ_REG
#define SOCKET_MSG_WRITE_REG				SERVER_MSG_WRITE_REG
#define SOCKET_MSG_SET_FILTER				SERVER_MSG_SET_FILTER
#define SOCKET_MSG_BEGIN_LISTEN_DATA		SERVER_MSG_BEGIN_LISTEN_DATA
#define SOCKET_MSG_END_LISTEN_DATA			SERVER_MSG_END_LISTEN_DATA
#define SOCKET_MSG_LISTEN_RAW_DATA			SERVER_MSG_LISTEN_RAW_DATA
#define SOCKET_MSG_LISTEN_FILTER_DATA		SERVER_MSG_LISTEN_FILTER_DATA

#define SOCKET_MSG_REPORT_REG				SERVER_MSG_REPORT_REG
#define SOCKET_MSG_REPORT_FILTER_CONFIG		SERVER_MSG_REPORT_FILTER_CONFIG
#define SOCKET_MSG_REPORT_LISTEN_DATA		SERVER_MSG_REPORT_LISTEN_DATA

int StartSocketServer(P_MSG_COM pMsgCom);
int CloseSocketServer(P_MSG_COM pMsgCom);

#endif /* SERVER_INC_SKTSERVER_H_ */
