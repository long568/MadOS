ifeq ($(LIB_UIP), yes)
export LIBS += -luip
export INCS += -I$(ROOT)/library/uIP/uip -I$(ROOT)/library/uIP/uip-funs
endif

ifeq ($(LIB_FATFS), yes)
export LIBS += -lfatfs
export INCS += -I$(ROOT)/library/FatFs
endif

ifeq ($(LIB_CJSON), yes)
export LIBS += -lcjson
export INCS += -I$(ROOT)/library/cJSON
endif

ifeq ($(LIB_LUA), yes)
export LIBS += -llua
export INCS += -I$(ROOT)/library/Lua
endif

ifeq ($(LIB_MODBUS), yes)
export LIBS += -lmodbus
export INCS += -I$(ROOT)/library/modbus
endif

ifeq ($(LIB_LWIP), yes)
export LIBS += -llwip
export INCS += -I$(ROOT)/library/LwIP/include -I$(ROOT)/library/LwIP/include/compat/posix
endif
