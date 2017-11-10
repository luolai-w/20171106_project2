#include "SGLocalConfig.h"
#include "SGLogTrace.h"
#include "SGConfigReader.h"

SGLocalConfig::SGLocalConfig()
{
    memset(m_acFileName,0,sizeof(m_acFileName));    
}

SGLocalConfig::~SGLocalConfig()
{
    
}

int SGLocalConfig::Initialize(const char* file)
{
    if (file == NULL)
    {
        return -1;
    } 

    strncpy(m_acFileName,file,sizeof(m_acFileName) -1);

    return 0;
}

int SGLocalConfig::LoadConfig(bool bReload)
{
    if (bReload)
    {
        return 0;
    }

    SGConfigReader confReader;

    int iRet = confReader.ReadFile(m_acFileName);
    if (iRet)
    {
        ERROR_LOG("ReadFile(%s) failed(%d)\n",m_acFileName,iRet);
        return -1;
    }

    confReader.GetKeyValue(NULL,"svr_id",m_iSvrID);
    confReader.GetKeyValue("SERVER","svr_type",m_iSvrType);
    confReader.GetKeyValue("SERVER","listen_port",m_iListenPort);
    confReader.GetKeyValue("SERVER","listen_ip",m_acListenIP,sizeof(m_acListenIP),"");
    
    int iHostCount = 0;
    confReader.GetKeyValue("LISTENER","game_host_count",iHostCount);
    for (int i = 0; i < iHostCount; i++)
    {
        struct HostInfo stHost;
        char szTmpKey[255] = {0};

        memset(&stHost,0,sizeof(struct HostInfo));

        snprintf(szTmpKey,sizeof(szTmpKey),"game_host%d",i);
        confReader.GetKeyValue("LISTENER",szTmpKey,stHost.m_acIP,sizeof(stHost.m_acIP));

        int iPort = 0;
        snprintf(szTmpKey,sizeof(szTmpKey),"game_port%d",i);
        confReader.GetKeyValue("LISTENER",szTmpKey,iPort);
        stHost.m_usPort = static_cast<unsigned short>(iPort);

        m_vcGameHosts.push_back(stHost); 
        
    }
     

    confReader.GetKeyValue("SERVER","server_conn_check_intvl",m_iServiceConnCheckIntvl,10);
    confReader.GetKeyValue("SERVER","server_conn_timeout",m_iServiceConnTimeout,30);
    confReader.GetKeyValue("SERVER","game_conn_check_intvl",m_iGameConnCheckIntvl,10);

    confReader.GetKeyValue("LUA","lua_time_update_intvl",m_iLuaTimeUpdateIntvl,1);
    confReader.GetKeyValue("LUA","lua_time_module",m_acLuaTimeModule,sizeof(m_acLuaTimeModule),"LTime");
    confReader.GetKeyValue("LUA","lua_time_update_function",m_acLuaTimeUpdateFunc,sizeof(m_acLuaTimeUpdateFunc),"Update");

    confReader.GetKeyValue("LUA","lua_main_path",m_acLuaMainPath,sizeof(m_acLuaMainPath),"../scripts/main.lua");

    return 0;
}

int SGLocalConfig::TraceConfig()
{
    ANY_LOG("svr_id:%d\n",m_iSvrID);
    ANY_LOG("svr_type:%d\n",m_iSvrType);
    ANY_LOG("listen_port:%d\n",m_iListenPort);
    ANY_LOG("listen_ip:%s\n",m_acListenIP);
    ANY_LOG("server_conn_check_intvl:%d\n",m_iServiceConnCheckIntvl);
    ANY_LOG("server_conn_timeout:%d\n",m_iServiceConnTimeout);
    ANY_LOG("game_conn_check_intvl:%d\n",m_iGameConnCheckIntvl);
    ANY_LOG("lua_time_update_intvl:%d\n",m_iLuaTimeUpdateIntvl);
    ANY_LOG("lua_main_path:%s\n",m_acLuaMainPath);
    ANY_LOG("lua_time_module:%s\n",m_acLuaTimeModule);
    ANY_LOG("lua_time_update_function:%s\n",m_acLuaTimeUpdateFunc);
 
    return 0;
}
