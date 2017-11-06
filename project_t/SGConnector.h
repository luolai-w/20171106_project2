#ifndef _SGCONNECTOR_H
#define _SGCONNECTOR_H

#include "SGObjectPool.h"

#define MAX_SEND_BUFF_SIZE (2*1024*1024)
#define MAX_RECV_BUFF_SIZE (1*1024*1024)


enum enSocketStatus
{
    SOCKET_STATUS_CLOSED,
    SOCKET_STATUS_CONNECTED,
    SOCKET_STATUS_ERRORk
};

enum enConnErrorReason{
    REASON_SEND_SOCKET_ERROR = -10,
    REASON_SEND_NO_BUFFER = -11,
    REASON_SEND_FAILED = -12,
    
    REASON_RECV_SOCKET_ERROR = -13,
    REASON_RECV_NO_BUFFER = -14,
    REASON_RECV_FAILED = -15
};

enum{
    PACKET_HEAD_LEN = sizeof(int)
};


class SGConnector:public SGObject{
    public:
        SGConnector();
        virtual ~SGConnector();
    
        int Initialize();
        int OnNew();
        int Destroy();

        int ConnectTo(const char* szIP, int iPort);
        int SetUpAcceptConnect(int iFD, int iPeerIP,unsigned short unPort);

        int OnSend();
        int Send(const char* pcData, int iLen);

        int OnRecv();
        int Recv(char* pcData, int& iLen);
        int HandlePacket();
        virtual int OnHandle(const char* pcData, int iLen);
        virtual int GetObjectTag()=0;

        int Close();
        int SetNonBlock(int iFD);

        int GetSocketFD(){ return m_iSocketFD; }
        bool IsConnected() { return m_iSocketStatus == SOCKET_STATUS_CONNECTED;}
    private:
        int m_iSocketFD;
        int m_iSocketStatus;
        int m_iPeerIP;
        int m_iPort;


        unsigned char* m_abSendBuffer[MAX_SEND_BUFF_SIZE];
        int m_iSendBegin;
        int m_iSendEnd;

        unsigned char* m_abRecvBuffer[MAX_RECV_BUFF_SIZE];
        int m_iRecvBegin;
        int m_iRecvEnd;
        
        bool m_bDestroyed;
};
#endif
