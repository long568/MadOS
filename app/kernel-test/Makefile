# $(ROOT)/app/$(APP)
export TEMP = $(BUILD_DIR)/app

ASMS =
SRCS = $(wildcard *.c)

include $(RULES)

after_all:
	$(LD) $(wildcard $(BUILD_DIR)/app/*.o) $(LDFLAGS) -o $(TARGET)
	$(OCPY) -O ihex $(TARGET) $(TARGET).hex
	$(ECHO) 'Building ... Done.'
