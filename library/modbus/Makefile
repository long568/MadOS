# $(ROOT)/library/modbus
LIBK = libmodbus.a
ASMS =
SRCS = $(wildcard *.c)

export TEMP = $(BUILD_DIR)/modbus

include $(RULES)

after_all:
	$(AR) rcs $(BUILD_DIR)/$(LIBK) $(wildcard $(TEMP)/*.o)
	$(ECHO) 'Creating $(LIBK) ... Done'
