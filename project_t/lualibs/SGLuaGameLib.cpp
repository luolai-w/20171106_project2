#include "SGLuaGameLib.h"
#include <sstream>
#include "SGLogTrace.h"
//#include <stdio.h>


static char sg_string_buffer[20*1024];
static char sg_stack_buffer[15*1024];


static const char* trace_stack_called_func(lua_State* L)
{
    lua_Debug ar;
    int iLevel = 1;

    if (!lua_getstack(L,iLevel,&ar))
    {
        return "";
    }

    if (!lua_getinfo(L,"Sln",&ar))
    {
        return "";
    }

    int iBuffLen = sizeof(sg_stack_buffer);
    sg_stack_buffer[iBuffLen -1] = 0;

    int iRet = snprintf(sg_stack_buffer,iBuffLen,"<%s:%d %s>",ar.short_src,ar.currentline,(ar.name == NULL? "unknow function name": ar.name)); 
    if (iRet < 0)
    {
        return "";
    }

    return sg_stack_buffer;
}

int sg_print(lua_State* L)
{
    int nArg = lua_gettop(L);
    if (nArg < 1)
    {
        return 0;
    }

    int iBuffLen = sizeof(sg_string_buffer);
    int iPos = 0;
    sg_string_buffer[iBuffLen -1] = 0;
    
    lua_getfield(L,LUA_GLOBALSINDEX,"tostring");
    for (int i =1; i <= nArg; i++)
    {
        const char* str = NULL;
        lua_pushvalue(L,-1);
        lua_pushvalue(L,i);
        lua_call(L,1,1);
        
        str = lua_tostring(L,-1);
        if (i > 1 )
        {
            int iRet = snprintf(sg_string_buffer+iPos,iBuffLen,"\t");
            if (iRet > 0)
            {
                iPos += iRet;
                iBuffLen -= iRet;

                if (iPos >= iBuffLen - 1)
                {
                    lua_pop(L,1);
                    break;
                } 
            } 
            
        }
        if (str == NULL)
        {
            ERROR_LOG("value at index %d can't to string\n",i);
            lua_pop(L,1);
            continue;
        } 

        int iRet = snprintf(sg_string_buffer+iPos,iBuffLen,"%s",str);
        if (iRet > 0)
        {
            iPos += iRet;
            iBuffLen -= iRet;

            if (iPos >= iBuffLen - 1)
            {
                lua_pop(L,1);
                break;
            }
        }

        lua_pop(L,1);
    }    

    lua_pop(L,1);

    ANY_LOG("%s %s\n",trace_stack_called_func(L),sg_string_buffer);    

    return 0;
}

int sg_errlog(lua_State* L)
{
    int nArg = lua_gettop(L);
    if (nArg < 1)
    {
        return 0;
    }

    int iBuffLen = sizeof(sg_string_buffer);
    int iPos = 0;
    sg_string_buffer[iBuffLen -1] = 0;
    
    lua_getfield(L,LUA_GLOBALSINDEX,"tostring");
    for (int i =1; i <= nArg; i++)
    {
        const char* str = NULL;
        lua_pushvalue(L,-1);
        lua_pushvalue(L,i);
        lua_call(L,1,1);
        
        str = lua_tostring(L,-1);
        if (i > 1 )
        {
            int iRet = snprintf(sg_string_buffer+iPos,iBuffLen,"\t");
            if (iRet > 0)
            {
                iPos += iRet;
                iBuffLen -= iRet;

                if (iPos >= iBuffLen - 1)
                {
                    lua_pop(L,1);
                    break;
                } 
            } 
            
        }
        if (str == NULL)
        {
            ERROR_LOG("value at index %d can't to string\n",i);
            lua_pop(L,1);
            continue;
        } 

        int iRet = snprintf(sg_string_buffer+iPos,iBuffLen,"%s",str);
        if (iRet > 0)
        {
            iPos += iRet;
            iBuffLen -= iRet;

            if (iPos >= iBuffLen - 1)
            {
                lua_pop(L,1);
                break;
            }
        }

        lua_pop(L,1);
    }    

    lua_pop(L,1);

    ERROR_LOG("%s %s\n",trace_stack_called_func(L),sg_string_buffer);    
    return 0;
}

static int __print_indent(std::ostream& o,int nNum)
{
    for (int i =0; i<nNum; i++ )
    {
        o << " ";
    }
    
    return 0;
}

static int __print_r(lua_State* L, int iTblIdx, int iToStringIdx, int iDepth, int iMaxDepth, std::ostream& o)
{
    __print_indent(o,iDepth*2); o << "{"<<std::endl;
    
    lua_pushnil(L);
    while(lua_next(L,iTblIdx))
    {
        int iValueIdx = lua_gettop(L);  //'value' at index -1
        int iKeyIdx = iValueIdx - 1;    //'key' at index -2

        lua_pushvalue(L,iToStringIdx);
        lua_pushvalue(L,iKeyIdx);
        lua_pcall(L,1,1,0);
        
        __print_indent(o,iDepth*2); o<<lua_tostring(L,-1)<<": ";
        
        lua_pop(L,1);
        
        if (lua_istable(L,iValueIdx) && iDepth < iMaxDepth )
        {
            __print_r(L,iValueIdx,iToStringIdx,iDepth+1,iMaxDepth, o);
        }else{

            lua_pushvalue(L,iToStringIdx);
            lua_pushvalue(L,iValueIdx);
            lua_pcall(L,1,1,0);
            
            __print_indent(o, iDepth*2); o <<lua_tostring(L,-1) <<std::endl;
             
            lua_pop(L,1);
            
        }

        lua_pop(L,1);
    } 
    
    __print_indent(o,iDepth*2); o <<"}"<<std::endl;

    return 0;
}

int tostring_r(lua_State* L)
{
    if (lua_gettop(L) == 0 || !lua_istable(L,1))
    {
        ERROR_LOG("tostring_r need first arguments is a table\n");
        return 0;
    }

    std::ostringstream oss;
    int iTblIdx = 1;
    int iDepth = 0;
    int iMaxDepth = 3;
    if (lua_gettop(L) >= 2)
    {
        iMaxDepth = lua_tonumber(L,2);
    }

    lua_getfield(L,LUA_GLOBALSINDEX,"tostring");

    int iToStringIdx = lua_gettop(L);
    
    __print_r(L,iTblIdx,iToStringIdx,iDepth, iMaxDepth, oss); 
    
    lua_pushstring(L,oss.str().c_str());  

    return 1;
}

int SetBitAt(lua_State* L)
{
    if (lua_gettop(L) >= 2 && lua_isnumber(L,1) && lua_isnumber(L,2))
    {
        unsigned int iValue = (unsigned int)lua_tonumber(L,1);
        unsigned int iOffset = (unsigned int)lua_tonumber(L,2);
        iValue = iValue | (1 << iOffset);
        
        lua_pushnumber(L,iValue);
        return 1;
    }
    
    luaL_error(L,"SetBitAt arguments check failed");
    return 0;
}

int ClearBitAt(lua_State* L)
{
    if (lua_gettop(L) >= 2 && lua_isnumber(L,1) && lua_isnumber(L,2))
    {
        unsigned int iValue = (unsigned int)lua_tonumber(L,1);
        unsigned int iOffset = (unsigned int)lua_tonumber(L,2);
        iValue = iValue & (~(1 << iOffset));

        lua_pushnumber(L,iValue);
        return 1;
    }

    luaL_error(L,"ClearBitAt arguments check failed");

    return 0;
}

int IsBitSet(lua_State* L)
{
    if (lua_gettop(L) >= 2 && lua_isnumber(L,1) && lua_isnumber(L,2))
    {
        bool bFlag = false;
        unsigned int iValue = (unsigned int)lua_tonumber(L,1);
        unsigned int iOffset = (unsigned int)lua_tonumber(L,2);
        
        if (iValue & (1 << iOffset))
        {
            bFlag = true;
        }

        lua_pushboolean(L,bFlag);

        return 1;
    }

    luaL_error(L,"IsBitSet arguments check failed");
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

    lua_pushcfunction(L, sg_print);
    lua_setfield(L, LUA_GLOBALSINDEX, "print");

    lua_pushcfunction(L, sg_errlog);
    lua_setfield(L, LUA_GLOBALSINDEX, "errlog");

    lua_pushcfunction(L,tostring_r);
    lua_setfield(L,LUA_GLOBALSINDEX,"tostring_r");

    
    luaL_register(L,"GameLib",(luaL_reg*)lua_lib);
    
    return 0;
}
