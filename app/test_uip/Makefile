# $(ROOT)/app/$(APP)
export TEMP = $(BUILD_DIR)/app

ASMS =
SRCS = $(wildcard *.c)

include $(RULES)

after_all:
	$(LD) $(wildcard $(BUILD_DIR)/app/*.o) -luip $(LDFLAGS) -o $(TARGET).elf
	$(OCPY) -O ihex $(TARGET).elf $(TARGET).hex
