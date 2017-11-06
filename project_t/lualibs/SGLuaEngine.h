#ifndef __SGLUAENGINE_H
#define __SGLUAENGINE_H
#include "lua.hpp"

class SGLuaEngine
{
public:
    SGLuaEngine();
    ~SGLuaEngine();

    int Initialize();
    int RunMainFile(const char* fileName,bool bReload);
    int DoFile(const char* fileName);
    lua_State* LuaState(){ return m_pLuaState;}
    
    static int RoutineLuaUpdate(void* pUserData); 

private:
    
   lua_State* m_pLuaState;     
};


#endif
