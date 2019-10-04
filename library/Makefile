# $(ROOT)/library
all:
	$(MAKE) -C $(ROOT)/library/Newlib
ifeq ($(LIB_UIP), yes)
	$(MAKE) -C $(ROOT)/library/uIP
endif
ifeq ($(LIB_FATFS), yes)
	$(MAKE) -C $(ROOT)/library/FatFs
endif
ifeq ($(LIB_CJSON), yes)
	$(MAKE) -C $(ROOT)/library/cJSON
endif
ifeq ($(LIB_LUA), yes)
	$(MAKE) -C $(ROOT)/library/Lua
endif
ifeq ($(LIB_MODBUS), yes)
	$(MAKE) -C $(ROOT)/library/modbus
endif
ifeq ($(LIB_LWIP), yes)
	$(MAKE) -C $(ROOT)/library/LwIP
endif
