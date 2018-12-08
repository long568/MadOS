# $(ROOT)/library
all:
	$(MAKE) -C $(ROOT)/library/Newlib
ifeq ($(LIB_UIP), yes)
	$(MAKE) -C $(ROOT)/library/uIP
endif
ifeq ($(LIB_FATFS), yes)
	$(MAKE) -C $(ROOT)/library/FatFs
endif
