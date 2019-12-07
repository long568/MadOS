# $(ROOT)/library/open62541
LIBK = libopen62541.a
ASMS =
SRCS = $(wildcard *.c)

export TEMP = $(BUILD_DIR)/open62541
export CFLAGS += -Wno-format -Wno-strict-aliasing

include $(RULES)

after_all:
	$(AR) rcs $(BUILD_DIR)/$(LIBK) $(wildcard $(TEMP)/*.o)
	$(ECHO) 'Creating $(LIBK) ... Done'
