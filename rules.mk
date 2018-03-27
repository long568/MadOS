DEPS = $(patsubst %.c, $(TEMP)/%.d, $(patsubst %.cpp, $(TEMP)/%.d, $(SRCS)))
OBJS = $(patsubst %.s, $(TEMP)/%.o, $(ASMS)) $(patsubst %.d, %.o, $(DEPS))

all : $(OBJS) after_all
	
include $(DEPS)

define gen_dep
	$(SET)
	$(ECHO) 'Scanning $< ...'
	$(RM) $@
	$(MKDIR) $(TEMP)
	@$(CC) $(CFLAGS) -MM $< > $@.tmp
	@sed 's,\($*.o\)[ :]*,$(TEMP)/\1 $@ : ,g' < $@.tmp > $@
	$(RM) $@.tmp
endef

$(TEMP)/%.d : %.c
	$(gen_dep)

$(TEMP)/%.d : %.cpp
	$(gen_dep)

$(TEMP)/%.o : %.s
	$(CC) $(CFLAGS) -c $< -o $@

$(TEMP)/%.o : %.c
	$(CC) $(CFLAGS) -c $< -o $@

$(TEMP)/%.o : %.cpp
	$(CPP) $(CPPFLAGS) -c $< -o $@

after_all: