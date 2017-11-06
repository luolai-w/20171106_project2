#include "SGLuaEngine.h"
#include "SGLocalConfig.h"
#include "SGMainCtl.h"
#include "SGLogTrace.h"
#include "SGLuaGameLib.h"


SGLuaEngine::SGLuaEngine():m_pLuaState(NULL)
{

}

SGLuaEngine::~SGLuaEngine()
{
   if (m_pLuaState != NULL)
    {
        lua_close(m_pLuaState);
        m_pLuaState = NULL;
    } 
}

int SGLuaEngine::Initialize()
{
    lua_State* L = luaL_newstate();
    if (L == NULL)
    {
        ERROR_LOG("lua_open() failed\n");
        return -1;
    } 
    
    luaL_openlibs(L);

    int iRet = SGLuaGameLib::Initialize(L);
    if (iRet)
    {
        ERROR_LOG("initialize SGLuaGameLib failed:%d\n",iRet);
        return -2;
    } 

    //todo

    lua_settop(L,0);

    m_pLuaState = L;

    g_pMainCtl->AddRoutineCheck(RoutineLuaUpdate,this,1); 
    return 0;
}

int SGLuaEngine::RunMainFile(const char* fileName, bool bReload)
{
    lua_State* L = LuaState();

    if (fileName == NULL || L == NULL)
    {
        return -1;
    }

    int iRet = luaL_loadfile(L,fileName);
    if (iRet)
    {
        ERROR_LOG("luaL_loadfile(L,%s) failed:%d(%s)\n",fileName,iRet,lua_tostring(L,-1));
        lua_settop(L,0);
        return -2;
    } 

    lua_pushboolean(L,bReload);
    iRet = lua_pcall(L,1,LUA_MULTRET,0);
    if (iRet)
    {
        ERROR_LOG("run chunk of %s failed:%d(%s)\n",fileName,iRet,lua_tostring(L,-1));
        lua_settop(L,0);
        return -3;
    }

    lua_settop(L,0); 

    return 0;
}

int SGLuaEngine::DoFile(const char* fileName)
{
    lua_State* L = LuaState();
    if (fileName == NULL || L == NULL)
    {
        return -1;
    }

    int iRet = luaL_dofile(L,fileName); 
    if (iRet)
    {
        ERROR_LOG("luaL_dofile(L,%s) failed:%d(%s)\n",fileName,iRet,lua_tostring(L,-1));
        lua_settop(L,0);
        return -2;
    }

    lua_settop(L,0);

    return 0;
}


int SGLuaEngine::RoutineLuaUpdate(void* pUserData)
{
    SGLuaEngine* pThis = reinterpret_cast<SGLuaEngine*>(pUserData);
    if (pThis == NULL)
    {
        return -1;    
    }

    lua_State* L = pThis->LuaState();
    if (L == NULL)
    {
        return -2;
    }

    unsigned int uiNowTime = g_pMainCtl->GetGameTime();
    lua_getfield(L,LUA_GLOBALSINDEX,g_pLocalConfig->m_acLuaTimeModule);
    lua_getfield(L,-1,g_pLocalConfig->m_acLuaTimeUpdateFunc);
    lua_pushinteger(L,uiNowTime);

    int iRet = lua_pcall(L,1,LUA_MULTRET,0);
    if (iRet)
    {
        ERROR_LOG("pcall %s.%s(%u) failed:%d(%s)\n",g_pLocalConfig->m_acLuaTimeModule,g_pLocalConfig->m_acLuaTimeUpdateFunc,uiNowTime,iRet,lua_tostring(L,-1));
        lua_settop(L,0);
        return -3;
    }  

    lua_settop(L,0);
    return 0;
}
