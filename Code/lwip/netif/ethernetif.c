/**
 * @file
 * Ethernet Interface Skeleton
 *
 */

/*
 * Copyright (c) 2001-2004 Swedish Institute of Computer Science.
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
 * Author: Adam Dunkels <adam@sics.se>
 *
 */

/*
 * This file is a skeleton for developing Ethernet network interface
 * drivers for lwIP. Add code to the low_level functions and do a
 * search-and-replace for the word "ethernetif" to replace it with
 * something that better describes your network interface.
 */

#include "lwip/opt.h"
#include "ENC28J60.h"
#include "UserConfig.h"

#if 1 /* don't build, this is only a skeleton, see previous comment */

#include "lwip/def.h"
#include "lwip/mem.h"
#include "lwip/pbuf.h"
#include <lwip/stats.h>
#include <lwip/snmp.h>
#include "netif/etharp.h"
#include "netif/ppp_oe.h"

/* Define those to better describe your network interface. */
#define IFNAME0 'e'
#define IFNAME1 'n'

#define EJ_DEV_DEBUG

/**
 * Helper struct to hold private data used to operate your ethernet interface.
 * Keeping the ethernet address of the MAC in this struct is not necessary
 * as it is already kept in the struct netif.
 * But this is only an example, anyway...
 */

/* Forward declarations. */
static void  ethernetif_input(struct netif *netif);
static void  eth_isr_thread(void *data);
static mad_bool_t resetRxLogic(struct ethernetif *ethif);
static mad_bool_t resetTxLogic(struct ethernetif *ethif);

/**
 * In this function, the hardware should be initialized.
 * Called from ethernetif_init().
 *
 * @param netif the already initialized lwip network interface structure
 *        for this ethernetif
 */
#undef EJ_DEV_TRY
#ifdef EJ_DEV_DEBUG
    #define EJ_DEV_TRY(x, res) MAD_TRY_2(x, res, ==, while(1) __asm volatile ( "NOP" ))
#else
    #define EJ_DEV_TRY(x, res) res = x
#endif
static void
low_level_init(struct netif *netif)
{
    mad_u8 res;
	struct ethernetif *ethif = netif->state;

	/* set MAC hardware address length */
	netif->hwaddr_len = ETHARP_HWADDR_LEN;

	/* set MAC hardware address */
	netif->hwaddr[0] = EJ_MAC_ADDR1;
	netif->hwaddr[1] = EJ_MAC_ADDR2;
	netif->hwaddr[2] = EJ_MAC_ADDR3;
	netif->hwaddr[3] = EJ_MAC_ADDR4;
	netif->hwaddr[4] = EJ_MAC_ADDR5;
	netif->hwaddr[5] = EJ_MAC_ADDR6;

	/* maximum transfer unit */
	netif->mtu = 1500;

	/* device capabilities */
	/* don't set NETIF_FLAG_ETHARP if this device is not an ethernet one */
	netif->flags = NETIF_FLAG_BROADCAST | NETIF_FLAG_ETHARP;// | NETIF_FLAG_LINK_UP;
 
	/* Do whatever else is needed to initialize interface. */
    res = res;
    EJ_DEV_TRY(madSemCreate(1), ethif->dev_locker);
    EJ_DEV_TRY(madSemCreateCarefully(0, 1), ethif->isr_locker);
    EJ_DEV_TRY(enc28j60Init(ethif), res);
    ethif->txbuf_cnt = 0;
    ethif->txbuf_wr = 0;
    ethif->txbuf_st = 0;
    ethif->txbuf_loop = EJ_RX_BUFFER_HEAD;
    ethif->rxbuf_next = EJ_RX_BUFFER_HEAD;
    ethif->is_linked = MFALSE;
}

/**
 * This function should do the actual transmission of the packet. The packet is
 * contained in the pbuf that is passed to the function. This pbuf
 * might be chained.
 *
 * @param netif the lwip network interface structure for this ethernetif
 * @param p the MAC packet to send (e.g. IP packet including MAC addresses and type)
 * @return ERR_OK if the packet could be sent
 *         an err_t value if the packet couldn't be sent
 *
 * @note Returning ERR_MEM here if a DMA queue of your MAC is full can lead to
 *       strange results. You might consider waiting for space in the DMA queue
 *       to become availale since the stack doesn't retry to send a packet
 *       dropped because of memory failure (except for the TCP timers).
 */
#undef EJ_DEV_TRY
#undef EJ_DEV_SPI
#ifdef EJ_DEV_DEBUG
    #define EJ_DEV_TRY(x, res) MAD_TRY_2(x, res, ==, madSemRelease(&ethif->dev_locker); return ERR_MEM)
    #define EJ_DEV_SPI(x, res) MAD_TRY_2(x, res, ==, enc28j60SpiNssDisable(ethif); madSemRelease(&ethif->dev_locker); return ERR_MEM)
#else
    #define EJ_DEV_TRY(x, res) x
    #define EJ_DEV_SPI(x, res) x
#endif
static err_t
low_level_output(struct netif *netif, struct pbuf *p)
{
	struct ethernetif *ethif = netif->state;
#ifdef EJ_DEV_DEBUG
    mad_u8 tmp;
#endif
    mad_cpsr_t cpsr;
    mad_bool_t is_linked;
	struct pbuf *q;
    mad_bool_t is_buf_return;
    mad_u16 pkg_st, buf_return;
	mad_u16 tot_len;
    
    madEnterCritical(cpsr);
    is_linked = ethif->is_linked;
    madExitCritical(cpsr);
    if(!is_linked)
        return ERR_MEM;

#if ETH_PAD_SIZE
	pbuf_header(p, -ETH_PAD_SIZE); /* drop the padding word */
#endif
	madSemWait(&ethif->dev_locker, 0);
    tot_len = 0;
    for(q = p; q != NULL; q = q->next)
		tot_len += q->len;
    is_buf_return = MFALSE;
    tot_len += 10;
    
    if(!ethif->txbuf_cnt)
    {
        ethif->txbuf_st = 0;
        ethif->txbuf_wr = 0;
        ethif->txbuf_loop = EJ_RX_BUFFER_HEAD;
    }
    else
    {
        if(ethif->txbuf_wr < ethif->txbuf_st)
        {
            if(ethif->txbuf_wr + tot_len > ethif->txbuf_st)
            {
                madSemRelease(&ethif->dev_locker);
                return ERR_MEM;
            }
        }
        else if(ethif->txbuf_wr > ethif->txbuf_st)
        {
            if(tot_len > EJ_RX_BUFFER_HEAD - ethif->txbuf_wr)
            {
                if(tot_len > ethif->txbuf_st)
                {
                    madSemRelease(&ethif->dev_locker);
                    return ERR_MEM;
                }
                is_buf_return = MTRUE;
                buf_return = ethif->txbuf_wr;
                ethif->txbuf_wr = 0;
            }
        }
        else
        {
            madSemRelease(&ethif->dev_locker);
            return ERR_MEM;
        }
    }

    pkg_st = ethif->txbuf_wr + 2;
    
	EJ_DEV_TRY(enc28j60WriteReg(ethif, EJ_ADDR_EWRPTL, GET_L8BIT(pkg_st)), tmp);
	EJ_DEV_TRY(enc28j60WriteReg(ethif, EJ_ADDR_EWRPTH, GET_H8BIT(pkg_st)), tmp);
	enc28j60SpiNssEnable(ethif);
	EJ_DEV_SPI(enc28j60WriteTxSt(ethif), tmp);
	for(q = p; q != NULL; q = q->next) {
		EJ_DEV_SPI(enc28j60WriteTx(ethif, q->payload, q->len), tmp);
    }
	enc28j60SpiNssDisable(ethif);
    if(!ethif->txbuf_cnt)
    {
        EJ_DEV_TRY(enc28j60BitSetETH(ethif, EJ_ADDR_ECON1, 0x80), tmp);
        EJ_DEV_TRY(enc28j60BitClearETH(ethif, EJ_ADDR_ECON1, 0x80), tmp);
        EJ_DEV_TRY(enc28j60BitClearETH(ethif, EJ_ADDR_EIR, 0x0A), tmp);
        EJ_DEV_TRY(enc28j60WriteReg(ethif, EJ_ADDR_ETXSTL, GET_L8BIT(ethif->txbuf_st + 2)), tmp);
        EJ_DEV_TRY(enc28j60WriteReg(ethif, EJ_ADDR_ETXSTH, GET_H8BIT(ethif->txbuf_st + 2)), tmp);
        EJ_DEV_TRY(enc28j60WriteReg(ethif, EJ_ADDR_ETXNDL, GET_L8BIT(tot_len - 8)), tmp);
        EJ_DEV_TRY(enc28j60WriteReg(ethif, EJ_ADDR_ETXNDH, GET_H8BIT(tot_len - 8)), tmp);
        EJ_DEV_TRY(enc28j60BitSetETH(ethif, EJ_ADDR_ECON1, 0x08), tmp);
        ethif->tx_pkg_len = tot_len;
    }
    else
    {
        EJ_DEV_TRY(enc28j60WrBufU16(ethif, ethif->txbuf_wr, tot_len), tmp);
    }
    ethif->txbuf_wr += tot_len;
    if(ethif->txbuf_wr & 0x0001) 
        ethif->txbuf_wr += 1;
    if(MTRUE == is_buf_return)
        ethif->txbuf_loop = buf_return;
    ethif->txbuf_cnt++;
    madSemRelease(&ethif->dev_locker);
#if ETH_PAD_SIZE
	pbuf_header(p, ETH_PAD_SIZE); /* reclaim the padding word */
#endif
	LINK_STATS_INC(link.xmit);
	return ERR_OK;
}

/**
 * Should allocate a pbuf and transfer the bytes of the incoming
 * packet from the interface into the pbuf.
 *
 * @param netif the lwip network interface structure for this ethernetif
 * @return a pbuf filled with the received packet (including MAC header)
 *         NULL on memory error
 */
#undef EJ_DEV_TRY
#undef EJ_DEV_SPI
#ifdef EJ_DEV_DEBUG
    #define EJ_DEV_TRY(x, res) x
    #define EJ_DEV_SPI(x, res) MAD_TRY_2(x, res, ==, enc28j60SpiNssDisable(ethif); return 0)
#else
    #define EJ_DEV_TRY(x, res) x
    #define EJ_DEV_SPI(x, res) x
#endif
static struct pbuf *
low_level_input(struct netif *netif)
{
	struct ethernetif *ethif = netif->state;
#ifdef EJ_DEV_DEBUG
    mad_u8 tmp;
#endif
	struct pbuf *p, *q;
	u16_t len;
	mad_u8 head[6];
	
    EJ_DEV_TRY(enc28j60WriteReg(ethif, EJ_ADDR_ERDPTL, GET_L8BIT(ethif->rxbuf_next)), tmp);
	EJ_DEV_TRY(enc28j60WriteReg(ethif, EJ_ADDR_ERDPTH, GET_H8BIT(ethif->rxbuf_next)), tmp);
	enc28j60SpiNssEnable(ethif);
	EJ_DEV_SPI(enc28j60ReadRxSt(ethif, head), tmp);
	
    ethif->rxbuf_next = head[0] | ((mad_u16)head[1] << 8);
	len = head[2] | ((mad_u16)head[3] << 8);
    if((!(head[4] & 0x80)) || (EJ_FRAME_MAX_LEN < len) || (EJ_RX_BUFFER_TAIL < ethif->rxbuf_next))
    {
        enc28j60SpiNssDisable(ethif);
        resetRxLogic(ethif);
        return 0;
    }

#if ETH_PAD_SIZE
	len += ETH_PAD_SIZE; /* allow room for Ethernet padding */
#endif
	p = pbuf_alloc(PBUF_RAW, len, PBUF_POOL);
	if (p != NULL) {
#if ETH_PAD_SIZE
		pbuf_header(p, -ETH_PAD_SIZE); /* drop the padding word */
#endif
		for(q = p; q != NULL; q = q->next) {

            if(MFALSE == enc28j60ReadRx(ethif, q->payload, q->len))
            {
                pbuf_free(p); 
                p = 0; 
                break;
            }
		}
#if ETH_PAD_SIZE
		pbuf_header(p, ETH_PAD_SIZE); /* reclaim the padding word */
#endif
		LINK_STATS_INC(link.recv);
	} else {
		LINK_STATS_INC(link.memerr);
		LINK_STATS_INC(link.drop);
	}
	
	enc28j60SpiNssDisable(ethif);
    EJ_DEV_TRY(enc28j60WriteReg(ethif, EJ_ADDR_ERXRDPTL, head[0]), tmp);
    EJ_DEV_TRY(enc28j60WriteReg(ethif, EJ_ADDR_ERXRDPTH, head[1]), tmp);
    EJ_DEV_TRY(enc28j60BitSetETH(ethif, EJ_ADDR_ECON2, 0x40), tmp);
	return p;
}

/**
 * This function should be called when a packet is ready to be read
 * from the interface. It uses the function low_level_input() that
 * should handle the actual reception of bytes from the network
 * interface. Then the type of the received packet is determined and
 * the appropriate input function is called.
 *
 * @param netif the lwip network interface structure for this ethernetif
 */
static void
ethernetif_input(struct netif *netif)
{
//	struct ethernetif *ethernetif;
	struct eth_hdr *ethhdr;
	struct pbuf *p;

//	ethernetif = netif->state;

	/* move received packet into a new pbuf */
	p = low_level_input(netif);
	/* no packet could be read, silently ignore this */
	if (p == NULL) return;
	/* points to packet payload, which starts with an Ethernet header */
	ethhdr = p->payload;

	switch (htons(ethhdr->type)) {
	/* IP or ARP packet? */
	case ETHTYPE_IP:
	case ETHTYPE_ARP:
	#if PPPOE_SUPPORT
	/* PPPoE packet? */
	case ETHTYPE_PPPOEDISC:
	case ETHTYPE_PPPOE:
	#endif /* PPPOE_SUPPORT */
		/* full packet send to tcpip_thread to process */
		if (netif->input(p, netif)!=ERR_OK) {
			LWIP_DEBUGF(NETIF_DEBUG, ("ethernetif_input: IP input error\n"));
			pbuf_free(p);
			p = NULL;
		}
		break;

	default:
		pbuf_free(p);
		p = NULL;
		break;
	}
}

#undef EJ_DEV_TRY
#undef EJ_DEV_TRY0
#undef EJ_DEV_SPI
#ifdef EJ_DEV_DEBUG
    #define EJ_DEV_TRY(x, res) MAD_TRY_2(x, res, ==, break)
#else
    #define EJ_DEV_TRY(x, res) x
#endif
static void eth_isr_thread(void *data)
{
#ifdef EJ_DEV_DEBUG
    mad_u8 tmp;
#endif
    mad_u16 phir, txbuf_nd;
	mad_u8 reg, pkg_cnt;
	mad_uint_t bug_cnt = 0;
	struct netif *netif = (struct netif *)data;
    struct ethernetif *ethif = netif->state;
    
	while(1)
	{
		if(SET == GPIO_ReadInputDataBit(ethif->gpio_int, ethif->pin_int))
			madSemWait(&ethif->isr_locker, 300);
		
		madSemWait(&ethif->dev_locker, 0);

        do {
            EJ_DEV_TRY(enc28j60BitClearETH(ethif, EJ_ADDR_EIE, ETH_ISR_MASK), tmp);
            EJ_DEV_TRY(enc28j60ReadRegETH(ethif, EJ_ADDR_EIR, &reg), tmp);
            
            if(reg & 0x10) {
                EJ_DEV_TRY(enc28j60ReadRegPHY(ethif, EJ_ADDR_PHIR, &phir), tmp);
                EJ_DEV_TRY(enc28j60ReadRegPHY(ethif, EJ_ADDR_PHSTAT2, &phir), tmp);
                ethif->is_linked = (phir & 0x0400) ? MTRUE : MFALSE;
                if(MTRUE == ethif->is_linked) {
                    EJ_DEV_TRY(resetRxLogic(ethif), tmp);
                    EJ_DEV_TRY(resetTxLogic(ethif), tmp);
                }
                break;
            }
            
            // Fix Chip Bug
            if((!reg) && ethif->txbuf_cnt) {
                bug_cnt++;
                if(3 < bug_cnt)
                    reg |= 0x40;
            } else {
                bug_cnt = 0;
            }
            
            if(reg & 0x0A) {
                EJ_DEV_TRY(enc28j60BitSetETH(ethif, EJ_ADDR_ECON1, 0x80), tmp);
                EJ_DEV_TRY(enc28j60BitClearETH(ethif, EJ_ADDR_ECON1, 0x80), tmp);
                EJ_DEV_TRY(enc28j60BitClearETH(ethif, EJ_ADDR_EIR, 0x0A), tmp);
                ethif->txbuf_cnt--;
                if(ethif->txbuf_cnt) {
                    ethif->txbuf_st += ethif->tx_pkg_len;
                    if(ethif->txbuf_st & 0x0001)
                        ethif->txbuf_st += 1;
                    if(ethif->txbuf_loop == ethif->txbuf_st) {
                        ethif->txbuf_loop = EJ_RX_BUFFER_HEAD;
                        ethif->txbuf_st = 0;
                    }
                    EJ_DEV_TRY(enc28j60RdBufU16(ethif, ethif->txbuf_st, &ethif->tx_pkg_len), tmp);
                    txbuf_nd = ethif->txbuf_st + ethif->tx_pkg_len - 8;
                    EJ_DEV_TRY(enc28j60WriteReg(ethif, EJ_ADDR_ETXSTL, GET_L8BIT(ethif->txbuf_st + 2)), tmp);
                    EJ_DEV_TRY(enc28j60WriteReg(ethif, EJ_ADDR_ETXSTH, GET_H8BIT(ethif->txbuf_st + 2)), tmp);
                    EJ_DEV_TRY(enc28j60WriteReg(ethif, EJ_ADDR_ETXNDL, GET_L8BIT(txbuf_nd)), tmp);
                    EJ_DEV_TRY(enc28j60WriteReg(ethif, EJ_ADDR_ETXNDH, GET_H8BIT(txbuf_nd)), tmp);
                    EJ_DEV_TRY(enc28j60BitSetETH(ethif, EJ_ADDR_ECON1, 0x08), tmp);
                }
            }
            
            if(reg & 0x40) {
                while(1) {
                    EJ_DEV_TRY(enc28j60ReadRegETH(ethif, EJ_ADDR_EPKTCNT, &pkg_cnt), tmp);
                    if(pkg_cnt)
                        ethernetif_input(netif);
                    else
                        break;
                }
            }
            
            if(reg & 0x01) {
                EJ_DEV_TRY(resetRxLogic(ethif), tmp);
            }
            
        } while(0);
		
        enc28j60BitSetETH(ethif, EJ_ADDR_EIE, ETH_ISR_MASK);
		madSemRelease(&ethif->dev_locker);
        if(reg & 0x10) {
            (MTRUE == ethif->is_linked) ? netif_set_link_up(netif) : netif_set_link_down(netif);
        }
	}
}

#undef EJ_DEV_TRY
#undef EJ_DEV_SPI
#ifdef EJ_DEV_DEBUG
    #define EJ_DEV_TRY(x, res) MAD_TRY_2(x, res, ==, return MFALSE)
#else
    #define EJ_DEV_TRY(x, res) x
#endif
static mad_bool_t resetRxLogic(struct ethernetif *ethif)
{
#ifdef EJ_DEV_DEBUG
    mad_u8 tmp;
#endif
    mad_u8 pkg_cnt;
    
//    EJ_DEV_TRY(enc28j60BitClearETH(ethif, EJ_ADDR_ECON1, 0x04), tmp);
    EJ_DEV_TRY(enc28j60BitSetETH(ethif, EJ_ADDR_ECON1, 0x40), tmp);
    EJ_DEV_TRY(enc28j60BitClearETH(ethif, EJ_ADDR_ECON1, 0x40), tmp);
    EJ_DEV_TRY(enc28j60WriteReg(ethif, EJ_ADDR_ERXSTL, GET_L8BIT(EJ_RX_BUFFER_HEAD)), tmp);
    EJ_DEV_TRY(enc28j60WriteReg(ethif, EJ_ADDR_ERXSTH, GET_H8BIT(EJ_RX_BUFFER_HEAD)), tmp);
    EJ_DEV_TRY(enc28j60WriteReg(ethif, EJ_ADDR_ERXNDL, GET_L8BIT(EJ_RX_BUFFER_TAIL)), tmp);
    EJ_DEV_TRY(enc28j60WriteReg(ethif, EJ_ADDR_ERXNDH, GET_H8BIT(EJ_RX_BUFFER_TAIL)), tmp);
    EJ_DEV_TRY(enc28j60WriteReg(ethif, EJ_ADDR_ERXRDPTL, GET_L8BIT(EJ_RX_BUFFER_HEAD)), tmp);
    EJ_DEV_TRY(enc28j60WriteReg(ethif, EJ_ADDR_ERXRDPTH, GET_H8BIT(EJ_RX_BUFFER_HEAD)), tmp);
    EJ_DEV_TRY(enc28j60WriteReg(ethif, EJ_ADDR_ERDPTL, GET_L8BIT(EJ_RX_BUFFER_HEAD)), tmp);
    EJ_DEV_TRY(enc28j60WriteReg(ethif, EJ_ADDR_ERDPTH, GET_H8BIT(EJ_RX_BUFFER_HEAD)), tmp);
    EJ_DEV_TRY(enc28j60ReadRegETH(ethif, EJ_ADDR_EPKTCNT, &pkg_cnt), tmp);
    while(pkg_cnt)
    {
        EJ_DEV_TRY(enc28j60BitSetETH(ethif, EJ_ADDR_ECON2, 0x40), tmp);
        EJ_DEV_TRY(enc28j60ReadRegETH(ethif, EJ_ADDR_EPKTCNT, &pkg_cnt), tmp);
    }
    EJ_DEV_TRY(enc28j60BitClearETH(ethif, EJ_ADDR_EIR, 0x01), tmp);
    EJ_DEV_TRY(enc28j60BitSetETH(ethif, EJ_ADDR_ECON1, 0x04), tmp);
    ethif->rxbuf_next = EJ_RX_BUFFER_HEAD;
    return MTRUE;
}

static mad_bool_t resetTxLogic(struct ethernetif *ethif)
{
#ifdef EJ_DEV_DEBUG
    mad_u8 tmp;
#endif
    EJ_DEV_TRY(enc28j60BitSetETH(ethif, EJ_ADDR_ECON1, 0x80), tmp);
    EJ_DEV_TRY(enc28j60BitClearETH(ethif, EJ_ADDR_ECON1, 0x80), tmp);
//    EJ_DEV_TRY(enc28j60BitClearETH(ethif, EJ_ADDR_ESTAT, 0x02), tmp);
    EJ_DEV_TRY(enc28j60BitClearETH(ethif, EJ_ADDR_EIR, 0x0A), tmp);
    ethif->txbuf_cnt = 0;
    ethif->txbuf_loop = EJ_RX_BUFFER_HEAD;
    ethif->txbuf_st = 0;
    ethif->txbuf_wr = 0;
    ethif->tx_pkg_len = 0;
    return MTRUE;
}

/**
 * Should be called at the beginning of the program to set up the
 * network interface. It calls the function low_level_init() to do the
 * actual setup of the hardware.
 *
 * This function should be passed as a parameter to netif_add().
 *
 * @param netif the lwip network interface structure for this ethernetif
 * @return ERR_OK if the loopif is initialized
 *         ERR_MEM if private data couldn't be allocated
 *         any other err_t on error
 */
#undef EJ_DEV_TRY
#ifdef EJ_DEV_DEBUG
    #define EJ_DEV_TRY(x, res) MAD_TRY_2(x, res, ==, while(1) __asm volatile ( "NOP" ))
#else
    #define EJ_DEV_TRY(x, res) x
#endif
err_t
ethernetif_init(struct netif *netif)
{
#ifdef EJ_DEV_DEBUG
    madTCB_t *tmp;
#endif
	LWIP_ASSERT("netif != NULL", (netif != NULL));

	#if LWIP_NETIF_HOSTNAME
	/* Initialize interface hostname */
	netif->hostname = "MadOS";
	#endif /* LWIP_NETIF_HOSTNAME */

	/*
	* Initialize the snmp variables and counters inside the struct netif.
	* The last argument should be replaced with your link speed, in units
	* of bits per second.
	*/
	NETIF_INIT_SNMP(netif, snmp_ifType_ethernet_csmacd, LINK_SPEED_OF_YOUR_NETIF_IN_BPS);

	netif->name[0] = IFNAME0;
	netif->name[1] = IFNAME1;
	/* We directly use etharp_output() here to save a function call.
	* You can instead declare your own function an call etharp_output()
	* from it if you have to do some checks before sending (e.g. if link
	* is available...) */
	netif->output = etharp_output;
	netif->linkoutput = low_level_output;

	((struct ethernetif*)(netif->state))->ethaddr = (struct eth_addr *)&(netif->hwaddr[0]);

	/* initialize the hardware */
	
	low_level_init(netif);
	
	EJ_DEV_TRY(madThreadCreate(eth_isr_thread, (void *)netif, ENC28J60_ISR_THREAD_STACKKSIZE, THREAD_PRIO_LWIP_ISR), tmp);

	return ERR_OK;
}

#endif /* 0 */
