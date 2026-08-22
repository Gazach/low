#ifndef LUA_SERVER_H
#define LUA_SERVER_H

#include <stdio.h>
#include <lua.h>
#include <lauxlib.h>

#include "../server/server.h"

// server.run()
static int server_run(lua_State *L)
{
    const char *port = luaL_checkstring(L, 1);

    server(port);

    return 0;
}

// Create the server module
static int server_module(lua_State *L)
{
    lua_newtable(L);

    lua_pushcfunction(L, server_run); // server.run = server_run
    lua_setfield(L, -2, "run");

    return 1;
}

// Register "server" as a Lua module
static void server_register(lua_State *L)
{
    lua_getglobal(L, "package");
    lua_getfield(L, -1, "preload");

    lua_pushcfunction(L, server_module);
    lua_setfield(L, -2, "server");

    lua_pop(L, 2);
}

#endif