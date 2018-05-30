# $(ROOT)/library
all :
	$(MAKE) -C $(ROOT)/library/Newlib
ifeq ($(LIB_UIP), yes)
	$(MAKE) -C $(ROOT)/library/uIP
endif
	