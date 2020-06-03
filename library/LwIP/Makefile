# $(ROOT)/library/LwIP
LIBK = liblwip.a

export TEMP = $(BUILD_DIR)/LwIP

all: gen_objs
	$(AR) rcs $(BUILD_DIR)/$(LIBK) $(wildcard $(TEMP)/*.o)
	$(ECHO) 'Creating $(LIBK) ... Done'

gen_objs:
	$(MAKE) -C $(ROOT)/library/LwIP/api
	$(MAKE) -C $(ROOT)/library/LwIP/port
	$(MAKE) -C $(ROOT)/library/LwIP/core
	$(MAKE) -C $(ROOT)/library/LwIP/core/ipv4
	$(MAKE) -C $(ROOT)/library/LwIP/netif
	$(MAKE) -C $(ROOT)/library/LwIP/apps/lwiperf
	$(MAKE) -C $(ROOT)/library/LwIP/apps/mqtt
