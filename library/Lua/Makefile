# $(ROOT)/library/Lua
LIBK = liblua.a
ASMS =
SRCS = $(wildcard *.c)

export TEMP = $(BUILD_DIR)/lua

include $(RULES)

after_all:
	$(AR) rcs $(BUILD_DIR)/$(LIBK) $(wildcard $(TEMP)/*.o)
	$(ECHO) 'Creating $(LIBK) ... Done'
