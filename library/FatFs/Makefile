# $(ROOT)/library/FatFs
LIBK = libfatfs.a
ASMS =
SRCS = $(wildcard *.c)

export TEMP = $(BUILD_DIR)/fatfs

include $(RULES)

after_all:
	$(AR) rcs $(BUILD_DIR)/$(LIBK) $(wildcard $(TEMP)/*.o)
	$(ECHO) 'Creating $(LIBK) ... Done'
