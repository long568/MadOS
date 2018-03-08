# $(ROOT)/library/Newlib
ASMS =
SRCS = $(wildcard *.c)
LIBK = libnewlib.a

export TEMP = $(BUILD_DIR)/newlib

include $(RULES)

after_all:
	$(AR) rcs $(BUILD_DIR)/$(LIBK) $(wildcard $(TEMP)/*.o)
	$(ECHO) 'Creating $(LIBK) ... Done'
