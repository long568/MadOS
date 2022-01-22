# $(ROOT)/app/$(APP)
export TEMP = $(BUILD_DIR)/app

ASMS =
SRCS = $(wildcard *.c) $(wildcard *.cpp)

include $(RULES)

after_all: gen_depend
	$(LD) $(wildcard $(BUILD_DIR)/app/*.o) $(LDFLAGS) -o $(TARGET).elf
	$(OCPY) -O ihex $(TARGET).elf $(TARGET).hex

gen_depend:
	$(MAKE) -C $(ROOT)/app/$(APP)/MAX30105
