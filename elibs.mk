ifeq ($(LIB_UIP), yes)
export LIBS += -luip
export INCS += -I$(ROOT)/library/uIP/uip -I$(ROOT)/library/uIP/uip-funs
endif
