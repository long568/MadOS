#include <stdlib.h>
#include <string.h>
#include "CfgUser.h"
#include "flash.h"

MadBool flash_init(void)
{
    flash_cfg_load();
    return MTRUE;
}
