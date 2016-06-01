#ifndef __ENC28J60_DEV__H__
#define __ENC28J60_DEV__H__

#include "MadOS.h"

#define EJ_CMD_MASK   ((mad_u8)0xE0)
#define EJ_ADDR_MASK  ((mad_u8)0x1F)

#define EJ_CMD_RCR    ((mad_u8)0x00)
#define EJ_CMD_RBM    ((mad_u8)(0x20 | 0x1A))
#define EJ_CMD_WCR    ((mad_u8)0x40)
#define EJ_CMD_WBM    ((mad_u8)(0x60 | 0x1A))
#define EJ_CMD_BFS    ((mad_u8)0x80)
#define EJ_CMD_BFC    ((mad_u8)0xA0)
#define EJ_CMD_SC     ((mad_u8)0xFF)

enum {
    EJ_ADDR_RETAIN 		= ((mad_u16)0x001A),
    EJ_ADDR_EIE,
    EJ_ADDR_EIR,
    EJ_ADDR_ESTAT,
    EJ_ADDR_ECON2,
    EJ_ADDR_ECON1
};

enum {
    EJ_ADDR_ERDPTL 		= ((mad_u16)0x0000),
    EJ_ADDR_ERDPTH,
    EJ_ADDR_EWRPTL,
    EJ_ADDR_EWRPTH,
    EJ_ADDR_ETXSTL,
    EJ_ADDR_ETXSTH,
    EJ_ADDR_ETXNDL,
    EJ_ADDR_ETXNDH,
    EJ_ADDR_ERXSTL,
    EJ_ADDR_ERXSTH,
    EJ_ADDR_ERXNDL,
    EJ_ADDR_ERXNDH,
    EJ_ADDR_ERXRDPTL,
    EJ_ADDR_ERXRDPTH,
    EJ_ADDR_ERXWRPTL,
    EJ_ADDR_ERXWRPTH,
    EJ_ADDR_EDMASTL,
    EJ_ADDR_EDMASTH,
    EJ_ADDR_EDMANDL,
    EJ_ADDR_EDMANDH,
    EJ_ADDR_EDMADSTL,
    EJ_ADDR_EDMADSTH,
    EJ_ADDR_EDMACSL,
    EJ_ADDR_EDMACSH
};

enum {
    EJ_ADDR_ETH0 		= ((mad_u16)0x0100),
    EJ_ADDR_ETH1,
    EJ_ADDR_ETH2,
    EJ_ADDR_ETH3,
    EJ_ADDR_ETH4,
    EJ_ADDR_ETH5,
    EJ_ADDR_ETH6,
    EJ_ADDR_ETH7,
    EJ_ADDR_EPMM0,
    EJ_ADDR_EPMM1,
    EJ_ADDR_EPMM2,
    EJ_ADDR_EPMM3,
    EJ_ADDR_EPMM4,
    EJ_ADDR_EPMM5,
    EJ_ADDR_EPMM6,
    EJ_ADDR_EPMM7,
    EJ_ADDR_EPMCSL,
    EJ_ADDR_EPMCSH,
    EJ_ADDR_EPMOL 		= ((mad_u16)0x0114),
    EJ_ADDR_EPMOH,
    EJ_ADDR_EWOLIE, 	// Reserved in DS_en.
    EJ_ADDR_EWOLIR, 	// Reserved in DS_en.
    EJ_ADDR_ERXFCON,
    EJ_ADDR_EPKTCNT
};

enum {
    EJ_ADDR_MACON1      = ((mad_u16)0x0200),
    EJ_ADDR_MACON2,     // Reserved in DS_en.
    EJ_ADDR_MACON3,
    EJ_ADDR_MACON4,
    EJ_ADDR_MABBIPG,
    EJ_ADDR_MAIPGL      = ((mad_u16)0x0206),
    EJ_ADDR_MAIPGH,
    EJ_ADDR_MACLCON1,
    EJ_ADDR_MACLCON2,
    EJ_ADDR_MAMXFLL,
    EJ_ADDR_MAMXFLH,
    EJ_ADDR_MAPSUP      = ((mad_u16)0x020D), // Reserved in DS_en.
    EJ_ADDR_MICON       = ((mad_u16)0x0211), // Reserved in DS_en.
    EJ_ADDR_MICMD,
    EJ_ADDR_MIREGADR    = ((mad_u16)0x0214),
    EJ_ADDR_MIWRL       = ((mad_u16)0x0216),
    EJ_ADDR_MIWRH,
    EJ_ADDR_MIRDL,
    EJ_ADDR_MIRDH
};

enum { // Flow DS_en
    EJ_ADDR_MAADR5 		= ((mad_u16)0x0300),
    EJ_ADDR_MAADR6,
    EJ_ADDR_MAADR3,
    EJ_ADDR_MAADR4,
    EJ_ADDR_MAADR1,
    EJ_ADDR_MAADR2,
    EJ_ADDR_EBSTSD,
    EJ_ADDR_EBSTCON,
    EJ_ADDR_EBSTCSL,
    EJ_ADDR_EBSTCSH,
    EJ_ADDR_MISTAT,
    EJ_ADDR_EREVID 		= ((mad_u16)0x0312),
    EJ_ADDR_ECOCON 		= ((mad_u16)0x0315),
    EJ_ADDR_EFLOCON 	= ((mad_u16)0x0317),
    EJ_ADDR_EPAUSL,
    EJ_ADDR_EPAUSH
};

enum {
    EJ_ADDR_PHCON1 = ((mad_u8)0x00),
    EJ_ADDR_PHSTAT1,
    EJ_ADDR_PHID1,
    EJ_ADDR_PHID2,
    EJ_ADDR_PHCON2 = ((mad_u8)0x10),
    EJ_ADDR_PHSTAT2,
    EJ_ADDR_PHIE,
    EJ_ADDR_PHIR,
    EJ_ADDR_PHLCON
};

#define PHY_CLOCK_READY  (0x01)
#define MII_START_RD     (0x01)
#define MII_BUSY         (0x01)
#define PHY_PDPXMD       (0x0100)
#define ETH_ISR_MASK     (0xDB)

#endif
