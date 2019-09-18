########################################################################
# DEX Files
########################################################################

DEX += _jit
DEX_SOURCES__jit += $(wildcard $(SRC_ROOT)/src/dex/_jit/*.c)
DEX_OBJECTS__jit = $(foreach F,$(DEX_SOURCES__jit),$(BLD_PATH)/dex/_jit/$(call relpath,$(F:.c=.o),$(SRC_ROOT)))

$(BLD_PATH)/dex/_jit/%.o: $(SRC_ROOT)/%.c
	@mkdir -p $(dir $@)
	$(CC_DEX) -MMD -MF $(@:.o=.MF) -c -o $@ $(CFLAGS) $(DEX_CFLAGS) $<
$(BIN_PATH)/lib/_jit$(DLL): $(DEX_OBJECTS__jit) $(BIN_PATH)/deemon$(EXE)
	$(CC_DEX) -o $@ -L$(BIN_PATH) $(LDFLAGS) $(DEX_LDFLAGS) $(DEX_OBJECTS__jit) $(LIBS) -ldeemon $(DEX_LIBS)

# Include GCC's automatically generated source dependency files
-include $(foreach F,$(DEX_OBJECTS__jit),$(F:.o=.MF))

