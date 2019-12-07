DEPS = $(patsubst %.c, $(TEMP)/%.d, $(patsubst %.cpp, $(TEMP)/%.d, $(SRCS)))
OBJS = $(patsubst %.s, $(TEMP)/%.o, $(ASMS)) $(patsubst %.d, %.o, $(DEPS))

all: $(OBJS) after_all
	
include $(DEPS)

define gen_dep
	$(SET)
	$(ECHO) 'Scanning $< ...'
	$(RM) $@
	$(MKDIR) $(TEMP)
	@$(1) $(2) -MM $< > $@.tmp
	@sed 's,^\($*.o\)[ :]*,$(TEMP)/\1 $@: ,g' < $@.tmp > $@
	$(RM) $@.tmp
endef

$(TEMP)/%.d: %.c
	$(call gen_dep, $(CC) $(CFLAGS))

$(TEMP)/%.d: %.cpp
	$(call gen_dep, $(CXX) $(CXXFLAGS))

$(TEMP)/%.o: %.s
	$(CC) $(CFLAGS) -c $< -o $@

$(TEMP)/%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

$(TEMP)/%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

after_all:
