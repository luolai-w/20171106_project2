#include "SGLuaGameLib.h"

int sg_print(lua_State* L)
{
    return 0;
}

int sg_errlog(lua_State* L)
{
    return 0;
}


int SetBitAt(lua_State* L)
{
    return 0;
}

int ClearBitAt(lua_State* L)
{
    return 0;
}

int IsBitSet(lua_State* L)
{
    return 0;
}

static luaL_Reg lua_lib[] = {
    {"SetBitAt",SetBitAt},
    {"ClearBitAt",ClearBitAt},
    {"IsBitSet",IsBitSet},
    {NULL,NULL}
};

int SGLuaGameLib::Initialize(lua_State* L)
{
    lua_getfield(L, LUA_GLOBALSINDEX, "print");
    lua_setfield(L, LUA_GLOBALSINDEX, "sg_print");        
    lua_pop(L,1);

    lua_pushcfunction(L, sg_print);
    lua_setfield(L, LUA_GLOBALSINDEX, "print");

    lua_pushcfunction(L, sg_errlog);
    lua_setfield(L, LUA_GLOBALSINDEX, "errlog");

    
    luaL_register(L,"GameLib",(luaL_reg*)lua_lib);
    
    return 0;
}
