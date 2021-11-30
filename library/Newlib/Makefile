# $(ROOT)/library/Newlib
LIBK = libnewlib.a
ASMS =
SRCS = $(wildcard *.c) $(wildcard *.cpp)

export TEMP = $(BUILD_DIR)/newlib

include $(RULES)

after_all:
	$(AR) rcs $(BUILD_DIR)/$(LIBK) $(wildcard $(TEMP)/*.o)
	$(ECHO) 'Creating $(LIBK) ... Done'
