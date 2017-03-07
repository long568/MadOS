#include "mod_lua.h"

#define MAD_LUA_STACK_SIZE (3 * 1024)

const char lua_name[] = "lua";
const char *lua_argv[2] = {lua_name, 0};
const int  lua_argc = sizeof(lua_argv) / sizeof(int) - 1;

extern int  madLuaMain (int argc, char **argv);
static void threadLua  (MadVptr exData);

static long long mad_lua_stk[MAD_LUA_STACK_SIZE / 8]; // Fix bug of Cortex-Mxs.

void initLua(void)
{
//    madThreadCreate(threadLua, 0, MAD_LUA_STACK_SIZE, THREAD_PRIO_LUA);
    madThreadCreateCarefully(threadLua, 0, MAD_LUA_STACK_SIZE, (void*)mad_lua_stk, THREAD_PRIO_LUA);
}

static void threadLua(MadVptr exData)
{
    (void)exData;
    while(1) {
        madLuaMain(lua_argc, (char **)lua_argv);
        madTimeDly(3000);
    }
}
