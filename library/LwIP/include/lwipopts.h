/*
 * Copyright (c) 2001-2003 Swedish Institute of Computer Science.
 * All rights reserved. 
 * 
 * Redistribution and use in source and binary forms, with or without modification, 
 * are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice,
 *    this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution.
 * 3. The name of the author may not be used to endorse or promote products
 *    derived from this software without specific prior written permission. 
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR IMPLIED 
 * WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF 
 * MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT 
 * SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, 
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT 
 * OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS 
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN 
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING 
 * IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY 
 * OF SUCH DAMAGE.
 *
 * This file is part of the lwIP TCP/IP stack.
 * 
 * Author: Simon Goldschmidt
 *
 */
#ifndef LWIP_HDR_LWIPOPTS_H
#define LWIP_HDR_LWIPOPTS_H

#include "MadOS.h"
#include "CfgUser.h"
#include "eth_low.h"

#define NO_SYS                          0
#define LWIP_STATS                      0
#define LWIP_NETCONN                    !NO_SYS
#define LWIP_SOCKET                     !NO_SYS
#define LWIP_NETBUF_RECVINFO            1
#define LWIP_HAVE_LOOPIF                1

#define LWIP_DHCP                       1
#define LWIP_DNS                        1
#define LWIP_NETIF_LINK_CALLBACK        1
#define LWIP_NETIF_HOSTNAME             1
#define LWIP_TIMEVAL_PRIVATE            0
#define LWIP_COMPAT_SOCKETS             3
#define LWIP_SOCKET_POLL                0
#define LWIP_SOCKET_SELECT              1 // Implemented by MadOS

#define ETH_PAD_SIZE                    2
#define MEM_ALIGNMENT                   MAD_MEM_ALIGN
#define MEMP_NUM_PBUF                   8
#define PBUF_POOL_SIZE                  8
#define MEM_LIBC_MALLOC                 1
#define MEMP_MEM_MALLOC                 1

#define TCP_QUEUE_OOSEQ                 0
#define TCP_MSS                         (ETH_PAYLOAD_LEN - 40)

#define TCPIP_MBOX_SIZE                 4
#define DEFAULT_RAW_RECVMBOX_SIZE       2
#define DEFAULT_UDP_RECVMBOX_SIZE       2
#define DEFAULT_TCP_RECVMBOX_SIZE       2
#define DEFAULT_ACCEPTMBOX_SIZE         2

#define TCPIP_THREAD_PRIO               THREAD_PRIO_LWIP_TCPIP
#define TCPIP_THREAD_STACKSIZE          1024

#define CHECKSUM_BY_HARDWARE            mEth_CHECKSUM_BY_HARDWARE
#if CHECKSUM_BY_HARDWARE
  #define CHECKSUM_GEN_IP               0
  #define CHECKSUM_GEN_UDP              0
  #define CHECKSUM_GEN_TCP              0
  #define CHECKSUM_GEN_ICMP             0
  #define CHECKSUM_GEN_ICMP6            0
  #define CHECKSUM_CHECK_IP             0
  #define CHECKSUM_CHECK_UDP            0
  #define CHECKSUM_CHECK_TCP            0
  #define CHECKSUM_CHECK_ICMP           0
  #define CHECKSUM_CHECK_ICMP6          0
#else
  #define CHECKSUM_GEN_IP               1
  #define CHECKSUM_GEN_UDP              1
  #define CHECKSUM_GEN_TCP              1
  #define CHECKSUM_GEN_ICMP             1
  #define CHECKSUM_GEN_ICMP6            1
  #define CHECKSUM_CHECK_IP             1
  #define CHECKSUM_CHECK_UDP            1
  #define CHECKSUM_CHECK_TCP            1
  #define CHECKSUM_CHECK_ICMP           1
  #define CHECKSUM_CHECK_ICMP6          1
#endif

// #define LWIP_DEBUG
#ifdef LWIP_DEBUG
  // #define PBUF_DEBUG         LWIP_DBG_ON
  // #define MEM_DEBUG          LWIP_DBG_ON
  // #define MEMP_DEBUG         LWIP_DBG_ON
  // #define NETIF_DEBUG        LWIP_DBG_ON
  // #define ICMP_DEBUG         LWIP_DBG_ON
  // #define IP_DEBUG           LWIP_DBG_ON
  // #define TCPIP_DEBUG        LWIP_DBG_ON
  // #define UDP_DEBUG          LWIP_DBG_ON
  // #define DHCP_DEBUG         LWIP_DBG_ON
#endif

#endif /* LWIP_HDR_LWIPOPTS_H */
