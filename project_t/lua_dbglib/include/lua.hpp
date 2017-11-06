// lua.hpp
// Lua header files for C++
// <<extern "C">> not supplied automatically because Lua also compiles as C++

extern "C" {
#include "lua.h"
#include "lualib.h"
#include "lauxlib.h"
}
extern int (luaL_error) (lua_State *L, const char *fmt, ...)  __attribute__ ((format (printf, 2, 3)));;
