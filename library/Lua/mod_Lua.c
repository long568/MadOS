#include <stdio.h>
#include <stdlib.h>
#include "MadOS.h"
#include "CfgUser.h"
#include "mod_Lua.h"

extern int lua_go (int argc, char *argv[]);

static char *argv[] = {
    "MadLua"
};

static void lua_parser_thread(MadVptr exData);

MadBool LuaParser_Init(void)
{
    if(MNULL == madThreadCreate(lua_parser_thread, MNULL, 1024 * 8, THREAD_PRIO_TEST_LUAPARSER)) {
        return MFALSE;
    } else {
        return MTRUE;
    }
}

static void lua_parser_thread(MadVptr exData)
{
    (void)exData;
    madTimeDly(100);
    while(1) {
        lua_go(1, argv);
        madThreadPend(MAD_THREAD_SELF);
    }
}
