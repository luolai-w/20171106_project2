#ifndef __SGLOCALCONFIG_H
#define __SGLOCALCONFIG_H

#include "SGSingleton.h"
#include <vector>

#define MAX_FILE_NAME_LENGTH 255
#define MAX_IPV4_LENGTH 32


struct HostInfo{
     unsigned short m_usPort;
     char m_acIP[MAX_IPV4_LENGTH];
};

class SGLocalConfig
{
    public:
        SGLocalConfig();
        ~SGLocalConfig();
        
        int Initialize(const char* file);
        int LoadConfig(bool bReload);
        int TraceConfig();
        
    public:
        char m_acFileName[MAX_FILE_NAME_LENGTH];        

        //param
        int m_iSvrID;
        int m_iSvrType;
        int m_iListenPort;
        char m_acListenIP[MAX_IPV4_LENGTH];

        int m_iServiceConnCheckIntvl;
        int m_iServiceConnTimeout;
        int m_iGameConnCheckIntvl;
    
        int m_iLuaTimeUpdateIntvl;
        char m_acLuaMainPath[MAX_FILE_NAME_LENGTH];
        char m_acLuaTimeModule[MAX_FILE_NAME_LENGTH];
        char m_acLuaTimeUpdateFunc[MAX_FILE_NAME_LENGTH];
        
        std::vector<struct HostInfo> m_vcGameHosts;  
};


typedef SGSingleton<SGLocalConfig> SGLocalConfigSgl;

#define g_pLocalConfig SGLocalConfigSgl::Instance()
#endif
