# $(ROOT)/driver/$(MCU_PREFIX)
ASMS =
SRCS = $(wildcard *.c)

include $(RULES)

after_all:
ifeq ($(MCU_PREFIX)_$(MCU_SUFFIX), stm32f10x_cl)
	$(MAKE) -C $(ROOT)/driver/$(MCU_PREFIX)/eth
endif
