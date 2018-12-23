# $(ROOT)/library/FatFs
LIBK = libcjson.a
ASMS =
SRCS = $(wildcard *.c)

export TEMP = $(BUILD_DIR)/cjson

include $(RULES)

after_all:
	$(AR) rcs $(BUILD_DIR)/$(LIBK) $(wildcard $(TEMP)/*.o)
	$(ECHO) 'Creating $(LIBK) ... Done'
