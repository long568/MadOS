#include "testLua.h"
#if LO_TEST_LUA

#include <stdio.h>
#include <stdlib.h>
#include "MadOS.h"
#include "CfgUser.h"
#include "lua.h"

static char *argv[] = {
    "lua",
    NULL
};

static void lua_parser_thread(MadVptr exData);

void Init_TestLua(void)
{
    if(MNULL == madThreadCreate(lua_parser_thread, MNULL, 1024 * 8, THREAD_PRIO_TEST_LUAPARSER)) {
        MAD_LOG("[Lua] LuaParser init failed!\n");
    }
}

static void lua_parser_thread(MadVptr exData)
{
    (void)exData;
    madTimeDly(1000);
    while(1) {
        lua(1, argv);
        MAD_LOG("[Lua] LuaParser exited!\n");
        madThreadDelete(MAD_THREAD_SELF);
    }
}

#endif /* LO_TEST_LUA */
