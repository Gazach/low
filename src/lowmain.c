#include <stdio.h>
#include <lua.h>
#include <lauxlib.h>
#include <lualib.h>

#include "bindings/lua_regis.h"

int main(int argc, char *argv[]) {
    if (argc < 2) {
        fprintf(stderr, "Usage: %s <script.lua>\n", argv[0]);
        return 1;
    }

    const char *script_path = argv[1];

    lua_State *L = luaL_newstate();
    luaL_openlibs(L);
    
    register_all_module(L);

    // Execute Lua script
    if (luaL_dofile(L, script_path) != LUA_OK) {
        printf("Lua error: %s\n", lua_tostring(L, -1));
        lua_close(L);
        return 1;
    }
    
    // Get the run function from Lua
    lua_getglobal(L, "run");

    // Check if run exists and is a function
    if (!lua_isfunction(L, -1)) {
        printf("Error: 'run()' function not found in script.\n");
        lua_pop(L, 1);
        lua_close(L);
        return 1;
    }

    // Call run()
    if (lua_pcall(L, 0, 0, 0) != LUA_OK) {
        printf("Error running function: %s\n", lua_tostring(L, -1));
        lua_pop(L, 1);
    }

    lua_close(L);
    return 0;
}