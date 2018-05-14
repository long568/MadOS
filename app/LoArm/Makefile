# $(ROOT)/app/$(APP)
export TEMP = $(BUILD_DIR)/app

ASMS =
SRCS = $(wildcard *.c)

include $(RULES)

after_all:
	$(LD) $(wildcard $(BUILD_DIR)/app/*.o) $(LDFLAGS) -luip -o $(TARGET).elf
	$(OCPY) -O ihex $(TARGET).elf $(TARGET).hex
