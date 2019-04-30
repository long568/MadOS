#include <stdio.h>
#include <string.h>
#include "mNetwork.h"
#include "uTcp.h"

// static uIP_App     TestUIP;
// static timer       timer_tcp;
// static struct pt   pt_tcp;
// static uIP_TcpConn *conn_tcp;
// static MadU8       linked;

// static void tcp_link_changed(MadVptr ep);
// static void tcp_start_up(void);
// static void tcp_shut_down(void);
// static void tcp_appcall(MadVptr ep);

// void Init_TestUIP(void)
// {
//     timer_init(&timer_tcp);
//     TestUIP.link_changed = tcp_link_changed;
//     uIP_AppRegister(&TestUIP);
// }

// void tcp_link_changed(MadVptr ep)
// {
//     linked = (MadU32)ep;
//     if(uIP_LINKED_OFF == linked) {
//         tcp_shut_down();
//     } else {
//         tcp_start_up();
//     }
// }

// void tcp_start_up(void)
// {
//     conn_tcp = uip_new();
//     if(conn_tcp) {
//         timer_add(&timer_tcp, &uIP_Clocker);
//         uIP_SetTcpConn(conn_tcp, tcp_appcall);
//         PT_INIT(&pt_tcp);
//     }
// }

// void tcp_shut_down(void)
// {
//     uIP_SetTcpConn(conn_tcp, MNULL); 
//     uip_remove(conn_tcp);
//     timer_remove(&timer_tcp);
// }

/*********************************************
 * tcp_appcall
 *********************************************/
// #define CHECK_IF_RESTART() \
// do {                                        \
//     if(TCP_FLAG_ERR == tcp_is_err()) {      \
//         uIP_SetTcpConn(conn_tcp, MNULL);    \
//         tcp_start_up();                     \
//         return PT_EXITED;                   \
//     }                                       \
// } while(0)

// static PT_THREAD(tcp_pt(MadVptr ep))
// {
//     static struct pt *pt = &pt_tcp;
//     PT_BEGIN(pt);
//     uip_ipaddr_t ipaddr;
//     SET_TARGET_IP(ipaddr);
//     uip_connect(&ipaddr, HTONS(5685));
//     PT_WAIT_UNTIL(pt, tcp_is_connected());
//     CHECK_IF_RESTART();
//     do {
//         PT_YIELD(pt);
//         CHECK_IF_RESTART();
//         if(uip_acked()) {
//         }
//         if(uip_newdata()) {
//         }
//         if(uip_rexmit()) {
//         }
//     } while(1);
//     PT_END(pt);
// }

// void tcp_appcall(MadVptr ep) { tcp_pt(ep); }
