# $(ROOT)/driver/$(MCU_PREFIX)
LIBK = libdrv.a
ASMS =
SRCS = $(wildcard *.c)

export TEMP = $(BUILD_DIR)/drv

include $(ROOT)/rules.mk

after_all:
	$(AR) rcs $(BUILD_DIR)/$(LIBK) $(wildcard $(TEMP)/*.o)
	$(ECHO) 'Creating $(LIBK) ... Done'
