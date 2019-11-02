# $(ROOT)/library/ENet
LIBK = libenet.a
ASMS =
SRCS = $(wildcard *.c)

export TEMP = $(BUILD_DIR)/ENet

include $(RULES)

after_all:
	$(AR) rcs $(BUILD_DIR)/$(LIBK) $(wildcard $(TEMP)/*.o)
	$(ECHO) 'Creating $(LIBK) ... Done'
