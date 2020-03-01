#include <stdarg.h>
#include <stdlib.h>
#include "MadDev.h"
#include "eth_low.h"

static int Drv_open  (const char *, int, va_list);
static int Drv_close (int fd);
static int Drv_ioctl (int fd, int request, va_list args);

const MadDrv_t MadDrvEther = {
    Drv_open,
    0,
    0,
    0,
    0,
    Drv_close,
    0,
    Drv_ioctl
};

static int Drv_open(const char * file, int flag, va_list args)
{
    int      rc    = 1;
    int      fd    = (int)file;
    MadDev_t *dev  = DevsList[fd];
    mEth_t   *port = (mEth_t *)(dev->port);
    const mEth_InitData_t *initData = (const mEth_InitData_t*)(dev->args->lowArgs);

    mEth_Callback_t fn = va_arg(args, mEth_Callback_t);
    MadVptr         ep = va_arg(args, MadVptr);

    if(!eth_low_init(port, initData, fn, ep)) {
        eth_low_deinit(port);
        rc = -1;
    }

    return rc;
}

static int Drv_close(int fd)
{
    MadDev_t *dev  = DevsList[fd];
    mEth_t   *port = (mEth_t *)(dev->port);
    eth_low_deinit(port);
    return 0;
}

static int Drv_ioctl(int fd, int request, va_list args)
{
    (void)fd; (void)request; (void)args;
    return -1;
}
