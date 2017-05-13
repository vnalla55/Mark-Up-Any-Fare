
EXE = simple-tcp-proxy
SRCS = simple-tcp-proxy.c
OBJS = simple-tcp-proxy.o

all: $(EXE)

$(EXE): $(OBJS)
	@echo Linking $@
	@gcc -o $@ $(EXE_LINKFLAGS) $(OBJS) $(LIB_DIR) $(LIB)
	@chmod a+rx $@

$(OBJS): $(SRCS)
	@echo Compiling $<
	@gcc -c $(COVERAGE_FLAGS) $(CFLAGS_BASE) $(CC_INCS) $< -o $@
        
clean:
	@echo Deleting objects
	@rm -f *.o $(EXE)
