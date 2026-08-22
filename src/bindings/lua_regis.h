#ifndef LUA_REGIS_H
#define LUA_REGIS_H

#include "lua_server.h"

static void register_all_module(lua_State *L){
    server_register(L);
}

#endif