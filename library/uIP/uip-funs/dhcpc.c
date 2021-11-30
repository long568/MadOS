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
 * @(#)$Id: dhcpc.c,v 1.2 2006/06/11 21:46:37 adam Exp $
 */

#include <stdio.h>
#include <string.h>

#include "dhcpc.h"

#if UIP_CORE_APP_DHCP

#define STATE_INITIAL 0
#define STATE_SENDING 1
#define STATE_OFFER_RECEIVED 2
#define STATE_CONFIG_RECEIVED 3

static struct dhcpc_state s;

struct dhcp_msg
{
    u8_t op, htype, hlen, hops;
    u8_t xid[4];
    u16_t secs, flags;
    u8_t ciaddr[4];
    u8_t yiaddr[4];
    u8_t siaddr[4];
    u8_t giaddr[4];
    u8_t chaddr[16];
#ifndef UIP_CONF_DHCP_LIGHT
    u8_t sname[64];
    u8_t file[128];
#endif
    u8_t options[312];
};

#define BOOTP_BROADCAST 0x8000

#define DHCP_REQUEST 1
#define DHCP_REPLY 2
#define DHCP_HTYPE_ETHERNET 1
#define DHCP_HLEN_ETHERNET 6
#define DHCP_MSG_LEN 236

#define DHCPC_SERVER_PORT 67
#define DHCPC_CLIENT_PORT 68

#define DHCPDISCOVER 1
#define DHCPOFFER 2
#define DHCPREQUEST 3
#define DHCPDECLINE 4
#define DHCPACK 5
#define DHCPNAK 6
#define DHCPRELEASE 7

#define DHCP_OPTION_SUBNET_MASK 1
#define DHCP_OPTION_ROUTER 3
#define DHCP_OPTION_DNS_SERVER 6
#define DHCP_OPTION_HOSTNAME 12 // Added by long
#define DHCP_OPTION_REQ_IPADDR 50
#define DHCP_OPTION_LEASE_TIME 51
#define DHCP_OPTION_MSG_TYPE 53
#define DHCP_OPTION_SERVER_ID 54
#define DHCP_OPTION_REQ_LIST 55
#define DHCP_OPTION_MTU 57 // Added by long
#define DHCP_OPTION_END 255

static const u8_t xid[4] = {0xad, 0xde, 0x12, 0x23};
static const u8_t magic_cookie[4] = {99, 130, 83, 99};
/*---------------------------------------------------------------------------*/
static u8_t *
add_msg_type(u8_t *optptr, u8_t type)
{
    *optptr++ = DHCP_OPTION_MSG_TYPE;
    *optptr++ = 1;
    *optptr++ = type;
    return optptr;
}
/*---------------------------------------------------------------------------*/
static u8_t *
add_server_id(u8_t *optptr)
{
    *optptr++ = DHCP_OPTION_SERVER_ID;
    *optptr++ = 4;
    memcpy(optptr, (const void *)s.serverid, 4);
    return optptr + 4;
}
/*---------------------------------------------------------------------------*/
static u8_t *
add_req_ipaddr(u8_t *optptr)
{
    *optptr++ = DHCP_OPTION_REQ_IPADDR;
    *optptr++ = 4;
    memcpy(optptr, (const void *)s.ipaddr, 4);
    return optptr + 4;
}
/*---------------------------------------------------------------------------*/
static u8_t *
add_req_options(u8_t *optptr)
{
    *optptr++ = DHCP_OPTION_REQ_LIST;
    *optptr++ = 3;
    *optptr++ = DHCP_OPTION_SUBNET_MASK;
    *optptr++ = DHCP_OPTION_ROUTER;
    *optptr++ = DHCP_OPTION_DNS_SERVER;
    return optptr;
}
/*---------------------------------------------------------------------------*/
static u8_t *
add_end(u8_t *optptr)
{
    *optptr++ = DHCP_OPTION_END;
    return optptr;
}
/*---------------------------------------------------------------------------*/
static u8_t *add_mtu(u8_t *optptr)
{ // Added by long
    *optptr++ = DHCP_OPTION_MTU;
    *optptr++ = 2;
    *optptr++ = (MadU8)((UIP_BUFSIZE & 0xFF00) >> 8);
    *optptr++ = (MadU8)(UIP_BUFSIZE & 0x00FF);
    return optptr;
}
static u8_t *add_hostname(u8_t *optptr)
{ // Added by long
    u8_t len;
    u8_t *p_len;
    *optptr++ = DHCP_OPTION_HOSTNAME;
    p_len = optptr++;
    len = DHCP_HOST_NAMES((char *)optptr);
    *p_len = len;
    optptr += len;
    return optptr;
}
/*---------------------------------------------------------------------------*/
static void
create_msg(register struct dhcp_msg *m)
{
    m->op = DHCP_REQUEST;
    m->htype = DHCP_HTYPE_ETHERNET;
    m->hlen = s.mac_len;
    m->hops = 0;
    memcpy(m->xid, xid, sizeof(m->xid));
    m->secs = 0;
    m->flags = 0; //HTONS(BOOTP_BROADCAST); /*  Broadcast bit. */
    /*  uip_ipaddr_copy(m->ciaddr, uip_hostaddr);*/
    memcpy(m->ciaddr, uip_hostaddr, sizeof(m->ciaddr));
    memset(m->yiaddr, 0, sizeof(m->yiaddr));
    memset(m->siaddr, 0, sizeof(m->siaddr));
    memset(m->giaddr, 0, sizeof(m->giaddr));
    memcpy(m->chaddr, s.mac_addr, s.mac_len);
    memset(&m->chaddr[s.mac_len], 0, sizeof(m->chaddr) - s.mac_len);
#ifndef UIP_CONF_DHCP_LIGHT
    memset(m->sname, 0, sizeof(m->sname));
    memset(m->file, 0, sizeof(m->file));
#endif

    memcpy(m->options, magic_cookie, sizeof(magic_cookie));
}
/*---------------------------------------------------------------------------*/
static void
send_discover(void)
{
    u8_t *end;
    struct dhcp_msg *m = (struct dhcp_msg *)uip_appdata;

    create_msg(m);

    end = add_msg_type(&m->options[4], DHCPDISCOVER);
    end = add_mtu(end);
    end = add_req_options(end);
    end = add_end(end);

    uip_send(uip_appdata, end - (u8_t *)uip_appdata);
}
/*---------------------------------------------------------------------------*/
static void
send_request(void)
{
    u8_t *end;
    struct dhcp_msg *m = (struct dhcp_msg *)uip_appdata;

    create_msg(m);

    end = add_msg_type(&m->options[4], DHCPREQUEST);
    end = add_mtu(end);
    end = add_server_id(end);
    end = add_req_ipaddr(end);
    end = add_hostname(end);
    end = add_end(end);

    uip_send(uip_appdata, end - (u8_t *)uip_appdata);
}
/*---------------------------------------------------------------------------*/
static u8_t
parse_options(u8_t *optptr, int len)
{
    u8_t *end = optptr + len;
    u8_t type = 0;

    while (optptr < end)
    {
        switch (*optptr)
        {
        case DHCP_OPTION_SUBNET_MASK:
            memcpy((void *)s.netmask, optptr + 2, 4);
            break;
        case DHCP_OPTION_ROUTER:
            memcpy((void *)s.default_router, optptr + 2, 4);
            break;
        case DHCP_OPTION_DNS_SERVER:
            memcpy((void *)s.dnsaddr, optptr + 2, 4);
            break;
        case DHCP_OPTION_MSG_TYPE:
            type = *(optptr + 2);
            break;
        case DHCP_OPTION_SERVER_ID:
            memcpy((void *)s.serverid, optptr + 2, 4);
            break;
        case DHCP_OPTION_LEASE_TIME:
            memcpy((void *)s.lease_time, optptr + 2, 4);
            break;
        case DHCP_OPTION_END:
            return type;
        }

        optptr += optptr[1] + 2;
    }
    return type;
}
/*---------------------------------------------------------------------------*/
static u8_t
parse_msg(void)
{
    struct dhcp_msg *m = (struct dhcp_msg *)uip_appdata;

    if (m->op == DHCP_REPLY &&
        memcmp(m->xid, xid, sizeof(xid)) == 0 &&
        memcmp(m->chaddr, s.mac_addr, s.mac_len) == 0)
    {
        memcpy((void *)s.ipaddr, m->yiaddr, 4);
        return parse_options(&m->options[4], uip_datalen());
    }
    return 0;
}
/*---------------------------------------------------------------------------*/
static void dhcpc_request(void);

static u8_t dhcpc_wait_offer(void) {
    s.data_ok = ((uip_newdata() && (parse_msg() == DHCPOFFER)) ? 1 : 0);
    return s.data_ok;
}

static u8_t dhcpc_wait_ack(void) {
    s.data_ok = ((uip_newdata() && (parse_msg() == DHCPACK)) ? 1 : 0);
    return s.data_ok;
}

static PT_THREAD(dhcpc_appcall(MadVptr ep))
{
    u32_t lease_time;
    (void)ep;

    PT_BEGIN(&s.pt);
    PT_WAIT_UNTIL(&s.pt, uIP_is_linked);

    s.state = STATE_INITIAL;
    dhcpc_request();

    tmr_init(&s.timer);
    tmr_add(&s.timer, &uIP_Clocker);
    tmr_set(&s.timer, MadTicksPerSec * 2);
    PT_WAIT_UNTIL(&s.pt, tmr_expired(&s.timer));
    MAD_LOG("[DHCP] Startup...\n");

    s.state = STATE_SENDING;
    s.ticks = MadTicksPerSec;
    s.data_ok = 0;

    do {
        send_discover();
        tmr_set(&s.timer, s.ticks);
        PT_WAIT_UNTIL(&s.pt, dhcpc_wait_offer() || tmr_expired(&s.timer));
        if (s.data_ok)
            break;
        if (s.ticks < MadTicksPerSec * 60)
        {
            s.ticks *= 2;
        }
    } while (s.state != STATE_OFFER_RECEIVED);

    s.ticks = MadTicksPerSec;
    s.data_ok = 0;

    do {
        send_request();
        tmr_set(&s.timer, s.ticks);
        PT_WAIT_UNTIL(&s.pt, dhcpc_wait_ack() || tmr_expired(&s.timer));
        if (s.data_ok)
            break;
        if (s.ticks <= MadTicksPerSec * 10)
        {
            s.ticks += MadTicksPerSec;
        }
        else
        {
            PT_RESTART(&s.pt);
        }
    } while (s.state != STATE_CONFIG_RECEIVED);

    lease_time = ntohs(s.lease_time[0]) * 65536ul + ntohs(s.lease_time[1]);
    MAD_LOG("[DHCP] IP address %d.%d.%d.%d\n",
            uip_ipaddr1(s.ipaddr), uip_ipaddr2(s.ipaddr),
            uip_ipaddr3(s.ipaddr), uip_ipaddr4(s.ipaddr));
    MAD_LOG("[DHCP] Netmask %d.%d.%d.%d\n",
            uip_ipaddr1(s.netmask), uip_ipaddr2(s.netmask),
            uip_ipaddr3(s.netmask), uip_ipaddr4(s.netmask));
    MAD_LOG("[DHCP] DNS server %d.%d.%d.%d\n",
            uip_ipaddr1(s.dnsaddr), uip_ipaddr2(s.dnsaddr),
            uip_ipaddr3(s.dnsaddr), uip_ipaddr4(s.dnsaddr));
    MAD_LOG("[DHCP] Default router %d.%d.%d.%d\n",
            uip_ipaddr1(s.default_router), uip_ipaddr2(s.default_router),
            uip_ipaddr3(s.default_router), uip_ipaddr4(s.default_router));
    MAD_LOG("[DHCP] Lease expires in %d seconds\n", lease_time);
    dhcpc_configured((const struct dhcpc_state *)&s);
    uIP_is_configured = MTRUE;

    tmr_set(&s.timer, lease_time * DHCP_RESTART_DIV);
    PT_WAIT_UNTIL(&s.pt, tmr_expired(&s.timer) || !uIP_is_linked);

    tmr_remove(&s.timer);
    if(!uIP_is_linked)
        uIP_is_configured = MFALSE;
    PT_END(&s.pt);
}
/*---------------------------------------------------------------------------*/
void dhcpc_init()
{
    uip_ipaddr_t addr;
    uIP_Lock();
    s.mac_addr = uip_ethaddr.addr;
    s.mac_len = 6;
    uip_ipaddr(addr, 255, 255, 255, 255);
    s.conn = uip_udp_new(&addr, HTONS(DHCPC_SERVER_PORT));
    if (s.conn != NULL) {
        uip_udp_bind(s.conn, HTONS(DHCPC_CLIENT_PORT));
        s.conn->appstate.app_call = dhcpc_appcall;
        s.conn->appstate.ep       = &s;
        PT_INIT(&s.pt);
    }
    uIP_Unlock();
    if (s.conn != NULL) {
        MAD_LOG("[DHCP] Init... OK\n");
    } else {
        MAD_LOG("[DHCP] Init... Failed\n");
    }
}

void dhcpc_request(void)
{
    u16_t ipaddr[2];
    if (s.state == STATE_INITIAL)
    {
        uip_ipaddr(ipaddr, 0, 0, 0, 0);
        uip_sethostaddr(ipaddr);
    }
}

void dhcpc_configured(const struct dhcpc_state *ps)
{
    uip_sethostaddr(ps->ipaddr);
    uip_setdraddr(ps->default_router);
    uip_setnetmask(ps->netmask);
    uip_setdnsaddr(ps->dnsaddr);
}

#endif /* UIP_CORE_APP_DHCP */
