#ifndef __SGLUAGAMELIB_H
#define __SGLUAGAMELIB_H
#include "lua.hpp"

class SGLuaGameLib{
public:
    SGLuaGameLib(){}
    ~SGLuaGameLib(){}

    static int Initialize(lua_State* L);
};

#endif
