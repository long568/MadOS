/*
 * Copyright (c) 2005, Swedish Institute of Computer Science
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. Neither the name of the Institute nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE INSTITUTE AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE INSTITUTE OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 *
 * This file is part of the uIP TCP/IP stack
 *
 * @(#)$Id: dhcpc.h,v 1.3 2006/06/11 21:46:37 adam Exp $
 */
#ifndef __DHCPC_H__
#define __DHCPC_H__

#include "MadOS.h"
#include "timer.h"
#include "pt.h"
#include "mod_uIP.h"

#define DHCP_SHOW_RESULT     0
#define DHCP_RESTART_DIV     (5 * CLOCK_SECOND / 10)
#define DHCP_HOST_NAMES(buf) sprintf(buf, "MadOS v%d.%d", MAD_VER_MAJOR, MAD_VER_SUB)

struct dhcpc_state {
  struct pt pt;
  char state;
  uIP_UdpConn *conn;
  timer timer;
  u16_t ticks;
  const void *mac_addr;
  int mac_len;
  
  u8_t serverid[4];

  u16_t lease_time[2];
  u16_t ipaddr[2];
  u16_t netmask[2];
  u16_t dnsaddr[2];
  u16_t default_router[2];
    
  u8_t data_ok; //Added by long
  uIP_App app;  //Added by long
};

void dhcpc_init(void);
void dhcpc_configured(const struct dhcpc_state *s);

#endif /* __DHCPC_H__ */
