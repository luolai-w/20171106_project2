#include "SGConnector.h"
#include "SGConnsManager.h"
#include "SGEpollEventLoop.h"
#include "SGLogTrace.h"
#include "SGMainCtl.h"
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <arpa/inet.h>
#include <sys/socket.h>


SGConnector::SGConnector():m_iSocketFD(-1),
m_iSocketStatus(SOCKET_STATUS_CLOSED),
m_iPeerIP(0),
m_iPort(0)
{
    m_iSendBegin = m_iSendEnd = 0;
    m_iRecvBegin = m_iRecvEnd = 0;
    memset(m_abSendBuffer,0,sizeof(m_abSendBuffer));
    memset(m_abRecvBuffer,0,sizeof(m_abRecvBuffer));

    m_bDestroyed = false;
}


SGConnector::~SGConnector()
{
    Close();
}

int SGConnector::Initialize()
{
    OnNew();
    return 0;
}

int SGConnector::OnNew()
{
    if (m_bDestroyed)
    {
        m_bDestroyed = false;

        m_iSendBegin = m_iSendEnd = 0;
        m_iRecvBegin = m_iRecvEnd = 0;
        m_iSocketStatus = SOCKET_STATUS_CLOSED;
        m_iSocketFD = -1;
        m_iPeerIP = 0;
        m_iPort = 0;
        memset(m_abSendBuffer,0,sizeof(m_abSendBuffer));
        memset(m_abRecvBuffer,0,sizeof(m_abRecvBuffer));                
    }

    return 0;
}

int SGConnector::Destroy()
{
    if (!m_bDestroyed)
    {
         ANY_LOG("Destroy conn fd(%d)\n",m_iSocketFD); 

         Close();
         m_bDestroyed = true;
    }

    return 0;
}


int SGConnector::ConnectTo(const char* szIP, int iPort)
{
    if (m_bDestroyed)
    {
        ERROR_LOG("conn is Destroyed!\n");
        return -1;
    }

    int iFD = socket(AF_INET,SOCK_STREAM,IPPROTO_TCP);
    if (iFD < 0 )
    {
        ERROR_LOG("create socket failed(%d):%d\n",iFD, errno);
        return -2;
    }

    struct sockaddr_in addr;
    memset(&addr,0,sizeof(struct sockaddr_in));
    addr.sin_family = AF_INET;
    addr.sin_port = htons((unsigned short)iPort);
    if (szIP != NULL && strlen(szIP) > 0 )
    {
        addr.sin_addr.s_addr = inet_addr(szIP);
    }else{
        addr.sin_addr.s_addr = htonl(INADDR_ANY);
    }

    int ret = connect(iFD,(const struct sockaddr*)& addr,sizeof(struct sockaddr_in));
    if (ret < 0)
    {
        ERROR_LOG("connect(%d) %s %d failed(%d):%d\n",iFD,szIP,iPort,ret,errno);
        close(iFD);
        return -3;
    }
    
    m_iSocketFD = iFD;
    m_iSocketStatus = SOCKET_STATUS_CONNECTED;

    SetNonBlock(m_iSocketFD);

    g_pMainCtl->GetEventLoop()->AddEvent(m_iSocketFD, EVENT_READ,CONSTRUCT_OBJECT_MIXID(GetObjectTag(),GetObjectID()));

    ANY_LOG("A new Connect: ip(%s),port(%d),fd(%d),ret(%d)\n",szIP,iPort,iFD,ret);    
    return 0;
}

int SGConnector::SetUpAcceptConnect(int iFD,int iPeerIP, unsigned short unPort)
{
   
    if (m_bDestroyed)
    {
        ERROR_LOG("conn is Destroyed!\n");
        return -1;
    }

    m_iSocketFD = iFD;
    m_iSocketStatus = SOCKET_STATUS_CONNECTED;
    m_iPeerIP = iPeerIP;
    m_iPort = (int)unPort;

    SetNonBlock(m_iSocketFD);
    g_pMainCtl->GetEventLoop()->AddEvent(m_iSocketFD, EVENT_READ,CONSTRUCT_OBJECT_MIXID(GetObjectTag(),GetObjectID()));

    return 0;
}

int SGConnector::SetNonBlock(int iFD)
{
    
    if (m_bDestroyed)
    {
        ERROR_LOG("conn is Destroyed!\n");
        return -1;
    }

    int flag = fcntl(iFD,F_GETFL,0);
    if (flag < 0 )
    {
        ERROR_LOG("fcntl(%d) get flag failed(%d):%d\n",iFD,flag,errno);
        return -1;
    }    

    flag |= O_NONBLOCK;

    int ret = fcntl(iFD,F_SETFL,flag);
    if (ret < 0)
    {
        ERROR_LOG("fcntl(%d) set flag failed(%d):%d\n",iFD,ret,errno);
        return -2;
    }

    return 0;
}

int SGConnector::Close()
{
    if (m_iSocketFD < 0)
    {
       return -1; 
    }
    
    ANY_LOG("close a conn fd(%d)\n",m_iSocketFD);

    g_pMainCtl->GetEventLoop()->DelEvent(m_iSocketFD);

    close(m_iSocketFD);

    m_iSocketFD = -1;
    m_iSocketStatus = SOCKET_STATUS_CLOSED;

    return 0;
}


//收到可发送事件
int SGConnector::OnSend()
{
    
    if (m_bDestroyed)
    {
        ERROR_LOG("conn is Destroyed!\n");
        return -1;
    }

    if (m_iSocketStatus != SOCKET_STATUS_CONNECTED)
    {
        return REASON_SEND_SOCKET_ERROR;
    }

    int iLeftSize = m_iSendEnd - m_iSendBegin;
    
    int iRet = 0;

    do{
        if (iLeftSize == 0)
        {
            m_iSendEnd = m_iSendBegin = 0;
            g_pMainCtl->GetEventLoop()->ChgEvent(m_iSocketFD,EVENT_READ,NULL);
            return 0;
        }

        iRet = send(m_iSocketFD, m_abSendBuffer+m_iSendBegin, iLeftSize,0);
        
        if (iRet > 0)
        {
            m_iSendBegin += iRet;
            iLeftSize -= iRet;
        }else if(iRet == 0){

            //Close();
            return REASON_SEND_FAILED;

        }else if(errno != EWOULDBLOCK){
            
            //Close();
            ERROR_LOG("send error fd(%d),err(%d)\n",m_iSocketFD,errno); 
            return REASON_SEND_SOCKET_ERROR;
        }
    
    }while(iRet > 0);

    return 0;
}

int SGConnector::Send(const char* pcData, int iLen)
{
    if (m_bDestroyed)
    {
        ERROR_LOG("conn is Destroyed!\n");
        return -1;
    }

    if (pcData == NULL || iLen <= 0)
    {
        return REASON_SEND_FAILED;
    }

    if (m_iSocketStatus != SOCKET_STATUS_CONNECTED)
    {
        return REASON_SEND_SOCKET_ERROR;
    }
    
    int iPos = 0; 
    int iLeftSize = m_iSendEnd - m_iSendBegin;

    int iRet = 0;

    if (iLeftSize > 0)
    {
        do{
            if (iLeftSize == 0)
            {
                m_iSendBegin = m_iSendEnd = 0;
                break;
            } 

            iRet = send(m_iSocketFD, m_abSendBuffer + m_iSendBegin, iLeftSize,0);
            if (iRet > 0)
            {
                m_iSendBegin += iRet;
                iLeftSize -= iRet;
            }else if(iRet < 0 && errno != EWOULDBLOCK){
                
                //Close();
                ERROR_LOG("send error fd(%d),err(%d)\n",m_iSocketFD,errno);
                return REASON_SEND_SOCKET_ERROR;
            }
            
        }while(iRet > 0);
        
        if (iLeftSize > 0)
        {
            if (MAX_SEND_BUFF_SIZE - (m_iSendEnd - m_iSendBegin) < iLen)
            {
                return REASON_SEND_NO_BUFFER;
            }

            if ( MAX_SEND_BUFF_SIZE - m_iSendEnd < iLen && m_iSendBegin > 0)
            {
                 memmove(m_abSendBuffer,m_abSendBuffer + m_iSendBegin,m_iSendEnd - m_iSendBegin);
                 m_iSendEnd -= m_iSendBegin;
                 m_iSendBegin = 0;
            }

            memcpy(m_abSendBuffer + m_iSendEnd,pcData, iLen); 
            m_iSendEnd += iLen;

            return 0;
        }
    }
    
    iLeftSize = iLen;
    do{
        if (iLeftSize == 0)
        {
            return 0;
        }

        iRet = send(m_iSocketFD,pcData + iPos,iLeftSize,0);
        if (iRet > 0 )
        {
            iLeftSize -= iRet;
            iPos += iRet;
        }else if( iRet < 0 && errno != EWOULDBLOCK){

            //Close();

            ERROR_LOG("send error fd(%d),err(%d)\n",m_iSocketFD,errno);
            return  REASON_SEND_SOCKET_ERROR; 
        } 
    
    }while( iRet > 0);


    if( MAX_SEND_BUFF_SIZE - m_iSendEnd < iLeftSize)
    {
        if (m_iSendBegin > 0)
        {
            memmove(m_abSendBuffer, m_abSendBuffer + m_iSendBegin, m_iSendEnd - m_iSendBegin);
            m_iSendEnd -= m_iSendBegin;
            m_iSendBegin = 0;
        }
        
        if ( MAX_SEND_BUFF_SIZE - m_iSendEnd < iLeftSize)
        {
            return REASON_SEND_NO_BUFFER;
        }
        
    }
    
    memcpy(m_abSendBuffer+m_iSendEnd, pcData + iPos, iLeftSize);
    m_iSendEnd += iLeftSize;

    g_pMainCtl->GetEventLoop()->ChgEvent(m_iSocketFD,EVENT_READ | EVENT_WRITE,NULL);

    return 0;
}

//收到可接收事件
int SGConnector::OnRecv()
{
    if (m_bDestroyed)
    {
        ERROR_LOG("conn is Destroyed!\n");
        return -1;
    }

    if (m_iSocketStatus == SOCKET_STATUS_CLOSED)
    {
        ERROR_LOG("connect is closed fd(%d)\n",m_iSocketFD);
        return -1;
    }
    
    if (m_iRecvBegin == m_iRecvEnd)
    {
        m_iRecvBegin = m_iRecvEnd = 0;
    }

    
    if (m_iRecvEnd - m_iRecvBegin >= MAX_RECV_BUFF_SIZE)
    {
        return REASON_RECV_NO_BUFFER;
    }

    int iRet = 0;
    ANY_LOG("begin>recv buffer length:%d\n",m_iRecvEnd - m_iRecvBegin);//test

    do{
        if (MAX_RECV_BUFF_SIZE - m_iRecvEnd <= 0)
        {
           if (m_iRecvBegin > 0)
           {
                memmove(m_abRecvBuffer,m_abRecvBuffer+m_iRecvBegin,m_iRecvEnd - m_iRecvBegin);
                m_iRecvEnd -= m_iRecvBegin;
                m_iRecvBegin = 0;
           }else{
                
               return REASON_RECV_NO_BUFFER;
           } 
        }

        iRet = recv(m_iSocketFD, m_abRecvBuffer+m_iRecvEnd,MAX_RECV_BUFF_SIZE - m_iRecvEnd,0);

        if (iRet > 0 )
        {

            m_iRecvEnd += iRet;

        }else if(iRet == 0){//对方关闭连接
            
            //Close();

            return REASON_RECV_FAILED;
        }else if(errno != EWOULDBLOCK){
          
           //Close();
           ERROR_LOG("recv error fd(%d),err(%d)\n",m_iSocketFD,errno);

           return REASON_RECV_SOCKET_ERROR; 
        } 
    
    }while(iRet > 0 );
    ANY_LOG("end>recv buffer length:%d\n",m_iRecvEnd - m_iRecvBegin);//test

    return 0;
}

int SGConnector::Recv(char* pcData,int& iLen)
{
    if (m_bDestroyed)
    {
        ERROR_LOG("conn is Destroyed!\n");
        return -1;
    }

    return 0;
}

int SGConnector::HandlePacket()
{
    if (m_bDestroyed)
    {
        ERROR_LOG("conn is Destroyed!\n");
        return -1;
    }

    int iDataLength = m_iRecvEnd - m_iRecvBegin;

    while(iDataLength >= PACKET_HEAD_LEN) 
    {
        const char* pcTemp = reinterpret_cast<char*>(&m_abRecvBuffer[m_iRecvBegin]);
        int packet_len = *((int*)pcTemp);
        ANY_LOG("packet_len(%d) recvbuff(%d)\n",packet_len,iDataLength);//test
        if (iDataLength < packet_len + PACKET_HEAD_LEN)
        {
            return 0;   //half packet
        }

        if (iDataLength == MAX_RECV_BUFF_SIZE &&  packet_len + PACKET_HEAD_LEN > MAX_RECV_BUFF_SIZE)
        {
            ERROR_LOG("recv buff(%d) is not enough, packet(%d)\n",MAX_RECV_BUFF_SIZE,packet_len + PACKET_HEAD_LEN);  
            return -2;
        } 


        pcTemp += PACKET_HEAD_LEN;
        m_iRecvBegin += PACKET_HEAD_LEN;       

        int ret =  OnHandle(pcTemp, packet_len);
        if (ret)
        {
            ERROR_LOG("OnHandle failed(%d),packet(%d)\n",ret,packet_len);
        } 
        
        m_iRecvBegin += packet_len;       
        iDataLength = m_iRecvEnd - m_iRecvBegin;
 
    }

    if (m_iRecvBegin == m_iRecvEnd)
    {
       m_iRecvBegin = m_iRecvEnd = 0; 
    }

    return 0;
}

int SGConnector::OnHandle(const char* pcData, int iLen)
{
    if (m_bDestroyed)
    {
        ERROR_LOG("conn is Destroyed!\n");
        return -1;
    }

    ANY_LOG("----------------------recv OnHandle Begin(%d)----------------------------\n",iLen);

    if (iLen < 4)
    {
        return -2;
    }
    
    int cmd = *((int*)pcData);
    int pos = 4;
    int iLeftSize = iLen - pos;
    
    char acTempBuff[1024*10] = {0};
    if (iLeftSize > (int)sizeof(acTempBuff)-1)
    {
        iLeftSize = sizeof(acTempBuff) -1;
    }
    memcpy(acTempBuff,pcData+pos,iLeftSize);

    ANY_LOG("---------------recv message(%d): (%d)%s-----------------------\n",iLeftSize,cmd,acTempBuff);

    ANY_LOG("-------------------------recv OnHandle end-------------------------------\n");
    return 0;
}
