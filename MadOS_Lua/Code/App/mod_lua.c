#include "mod_lua.h"

const char lua_name[] = "lua";
const char *lua_argv[2] = {lua_name, 0};
const int  lua_argc = sizeof(lua_argv) / sizeof(int) - 1;

extern int  madLuaMain (int argc, char **argv);
static void threadLua  (MadVptr exData);

static long long mad_lua_stk[1024];

void initLua(void)
{
//    madThreadCreate(threadLua, 0, 8 * 1024, THREAD_PRIO_LUA);
    madThreadCreateCarefully(threadLua, 0, 8192, (void*)mad_lua_stk, THREAD_PRIO_LUA);
}

static void threadLua(MadVptr exData)
{
    (void)exData;
    while(1) {
        madLuaMain(lua_argc, (char **)lua_argv);
        madTimeDly(3000);
    }
}
