/*
 * Example:
 *     uTcp *tcp = new uTcp(192, 168, 1, 103, 5685);
 */

#ifndef __UIP_TCP__H__
#define __UIP_TCP__H__

#ifdef __cplusplus
extern "C"{

#include "pt.h"
#include "mod_uIP.h"

} /* extern "C" */
#endif

class uTcp {
public:
    uTcp(MadU8 ip[4], MadU16 port);
    uTcp(MadU8 ip0, MadU8 ip1, MadU8 ip2, MadU8 ip3, MadU16 port);

protected:
    enum {
        TCP_FLAG_OK = 0,
        TCP_FLAG_CON,
        TCP_FLAG_ERR
    };

    MadU8 isError    (void);
    MadU8 isConnected(void);
    void  startup    (void);
    void  shutdown   (void);

    static void linkChanged(MadVptr p, MadVptr ep);
    static PT_THREAD(appcall(MadVptr p, MadVptr ep));

private:
    uIP_App     m_app;
    uIP_TcpConn *m_conn;
    timer       m_timer;
    struct pt   m_pt;
    MadU8       m_isLinked;

    MadU8       m_ip[4];
    MadU16      m_port;
};

#endif
