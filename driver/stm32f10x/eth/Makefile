# $(ROOT)/driver/$(MCU_PREFIX)/eth
ASMS =
SRCS = $(wildcard *.c)

include $(RULES)

after_all:
ifeq ($(LIB_LWIP), yes)
	$(MAKE) -C $(ROOT)/driver/$(MCU_PREFIX)/eth/lwip
endif
ifeq ($(LIB_UIP), yes)
	$(MAKE) -C $(ROOT)/driver/$(MCU_PREFIX)/eth/uip
endif
