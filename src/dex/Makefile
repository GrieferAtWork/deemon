########################################################################
# DEX Files
########################################################################

# $(call dex_objects,_jit,$(DEX_SOURCES__jit))
override define dex_objects
$(foreach F,$2,$(BLD_PATH)/dex/$1/$(call relpath,$(F:.c=.o),$(SRC_ROOT)))
endef


### _jit
DEX += _jit
DEX_SOURCES__jit += $(wildcard $(SRC_ROOT)/src/dex/_jit/*.c)
DEX_OBJECTS__jit = $(call dex_objects,_jit,$(DEX_SOURCES__jit))
-include $(foreach F,$(DEX_OBJECTS__jit),$(F:.o=.MF))
$(BLD_PATH)/dex/_jit/%.o: $(SRC_ROOT)/%.c
	@mkdir -p $(dir $@)
	$(DEX_CC) -MMD -MF $(@:.o=.MF) -c -o $@ $(CFLAGS) $(DEX_CFLAGS) $<
$(BIN_PATH)/lib/_jit$(DLL): $(DEX_OBJECTS__jit) $(BIN_PATH)/deemon$(EXE)
	$(DEX_CC) -o $@ -L$(BIN_PATH) $(LDFLAGS) $(DEX_LDFLAGS) $(DEX_OBJECTS__jit) $(LIBS) $(DEX_LIBS)


### collections
DEX += collections
DEX_SOURCES_collections += $(wildcard $(SRC_ROOT)/src/dex/collections/*.c)
DEX_OBJECTS_collections = $(call dex_objects,collections,$(DEX_SOURCES_collections))
-include $(foreach F,$(DEX_OBJECTS_collections),$(F:.o=.MF))
$(BLD_PATH)/dex/collections/%.o: $(SRC_ROOT)/%.c
	@mkdir -p $(dir $@)
	$(DEX_CC) -MMD -MF $(@:.o=.MF) -c -o $@ $(CFLAGS) $(DEX_CFLAGS) $<
$(BIN_PATH)/lib/collections$(DLL): $(DEX_OBJECTS_collections) $(BIN_PATH)/deemon$(EXE)
	$(DEX_CC) -o $@ -L$(BIN_PATH) $(LDFLAGS) $(DEX_LDFLAGS) $(DEX_OBJECTS_collections) $(LIBS) $(DEX_LIBS)


## hashlib
DEX += hashlib
DEX_SOURCES_hashlib += $(wildcard $(SRC_ROOT)/src/dex/hashlib/*.c)
DEX_OBJECTS_hashlib = $(call dex_objects,hashlib,$(DEX_SOURCES_hashlib))
-include $(foreach F,$(DEX_OBJECTS_hashlib),$(F:.o=.MF))
$(BLD_PATH)/dex/hashlib/%.o: $(SRC_ROOT)/%.c
	@mkdir -p $(dir $@)
	$(DEX_CC) -MMD -MF $(@:.o=.MF) -c -o $@ $(CFLAGS) $(DEX_CFLAGS) $<
$(BIN_PATH)/lib/hashlib$(DLL): $(DEX_OBJECTS_hashlib) $(BIN_PATH)/deemon$(EXE)
	$(DEX_CC) -o $@ -L$(BIN_PATH) $(LDFLAGS) $(DEX_LDFLAGS) $(DEX_OBJECTS_hashlib) $(LIBS) $(DEX_LIBS)


### ctypes
ifndef CONFIG_WITHOUT_DEX_CTYPES
ifndef DEX_CTYPES_CC
DEX_CTYPES_CC := $(DEX_CC)
endif
DEX += ctypes
DEX_SOURCES_ctypes += $(wildcard $(SRC_ROOT)/src/dex/ctypes/*.c)
DEX_OBJECTS_ctypes = $(call dex_objects,ctypes,$(DEX_SOURCES_ctypes))
-include $(foreach F,$(DEX_OBJECTS_ctypes),$(F:.o=.MF))
$(BLD_PATH)/dex/ctypes/%.o: $(SRC_ROOT)/%.c
	@mkdir -p $(dir $@)
	$(DEX_CTYPES_CC) -MMD -MF $(@:.o=.MF) -c -o $@ $(CFLAGS) $(DEX_CFLAGS) $(DEX_CTYPES_CFLAGS) $<
$(BIN_PATH)/lib/ctypes$(DLL): $(DEX_OBJECTS_ctypes) $(BIN_PATH)/deemon$(EXE)
	$(DEX_CTYPES_CC) -o $@ -L$(BIN_PATH) $(LDFLAGS) $(DEX_LDFLAGS) $(DEX_CTYPES_LDFLAGS) $(DEX_OBJECTS_ctypes) $(LIBS) $(DEX_LIBS) $(DEX_CTYPES_LIBS)
endif


### disassembler
DEX += disassembler
DEX_SOURCES_disassembler += $(wildcard $(SRC_ROOT)/src/dex/disassembler/*.c)
DEX_OBJECTS_disassembler = $(call dex_objects,disassembler,$(DEX_SOURCES_disassembler))
-include $(foreach F,$(DEX_OBJECTS_disassembler),$(F:.o=.MF))
$(BLD_PATH)/dex/disassembler/%.o: $(SRC_ROOT)/%.c
	@mkdir -p $(dir $@)
	$(DEX_CC) -MMD -MF $(@:.o=.MF) -c -o $@ $(CFLAGS) $(DEX_CFLAGS) $<
$(BIN_PATH)/lib/disassembler$(DLL): $(DEX_OBJECTS_disassembler) $(BIN_PATH)/deemon$(EXE)
	$(DEX_CC) -o $@ -L$(BIN_PATH) $(LDFLAGS) $(DEX_LDFLAGS) $(DEX_OBJECTS_disassembler) $(LIBS) $(DEX_LIBS)


### files
DEX += files
DEX_SOURCES_files += $(wildcard $(SRC_ROOT)/src/dex/files/*.c)
DEX_OBJECTS_files = $(call dex_objects,files,$(DEX_SOURCES_files))
-include $(foreach F,$(DEX_OBJECTS_files),$(F:.o=.MF))
$(BLD_PATH)/dex/files/%.o: $(SRC_ROOT)/%.c
	@mkdir -p $(dir $@)
	$(DEX_CC) -MMD -MF $(@:.o=.MF) -c -o $@ $(CFLAGS) $(DEX_CFLAGS) $<
$(BIN_PATH)/lib/files$(DLL): $(DEX_OBJECTS_files) $(BIN_PATH)/deemon$(EXE)
	$(DEX_CC) -o $@ -L$(BIN_PATH) $(LDFLAGS) $(DEX_LDFLAGS) $(DEX_OBJECTS_files) $(LIBS) $(DEX_LIBS)


### ipc
ifndef CONFIG_WITHOUT_DEX_IPC
DEX += ipc
DEX_SOURCES_ipc += $(wildcard $(SRC_ROOT)/src/dex/ipc/*.c)
DEX_OBJECTS_ipc = $(call dex_objects,ipc,$(DEX_SOURCES_ipc))
-include $(foreach F,$(DEX_OBJECTS_ipc),$(F:.o=.MF))
$(BLD_PATH)/dex/ipc/%.o: $(SRC_ROOT)/%.c
	@mkdir -p $(dir $@)
	$(DEX_CC) -MMD -MF $(@:.o=.MF) -c -o $@ $(CFLAGS) $(DEX_CFLAGS) $<
$(BIN_PATH)/lib/ipc$(DLL): $(DEX_OBJECTS_ipc) $(BIN_PATH)/deemon$(EXE)
	$(DEX_CC) -o $@ -L$(BIN_PATH) $(LDFLAGS) $(DEX_LDFLAGS) $(DEX_OBJECTS_ipc) $(LIBS) $(DEX_LIBS)
endif


### json
ifndef CONFIG_WITHOUT_DEX_JSON
DEX += json
DEX_SOURCES_json += $(wildcard $(SRC_ROOT)/src/dex/json/*.c)
DEX_OBJECTS_json = $(call dex_objects,json,$(DEX_SOURCES_json))
-include $(foreach F,$(DEX_OBJECTS_json),$(F:.o=.MF))
$(BLD_PATH)/dex/json/%.o: $(SRC_ROOT)/%.c
	@mkdir -p $(dir $@)
	$(DEX_CC) -MMD -MF $(@:.o=.MF) -c -o $@ $(CFLAGS) $(DEX_CFLAGS) $<
$(BIN_PATH)/lib/json$(DLL): $(DEX_OBJECTS_json) $(BIN_PATH)/deemon$(EXE)
	$(DEX_CC) -o $@ -L$(BIN_PATH) $(LDFLAGS) $(DEX_LDFLAGS) $(DEX_OBJECTS_json) $(LIBS) $(DEX_LIBS)
endif


### math
ifndef CONFIG_WITHOUT_DEX_MATH
DEX += math
DEX_SOURCES_math += $(wildcard $(SRC_ROOT)/src/dex/math/*.c)
DEX_OBJECTS_math = $(call dex_objects,math,$(DEX_SOURCES_math))
-include $(foreach F,$(DEX_OBJECTS_math),$(F:.o=.MF))
$(BLD_PATH)/dex/math/%.o: $(SRC_ROOT)/%.c
	@mkdir -p $(dir $@)
	$(DEX_CC) -MMD -MF $(@:.o=.MF) -c -o $@ $(CFLAGS) $(DEX_CFLAGS) $<
$(BIN_PATH)/lib/math$(DLL): $(DEX_OBJECTS_math) $(BIN_PATH)/deemon$(EXE)
	$(DEX_CC) -o $@ -L$(BIN_PATH) $(LDFLAGS) $(DEX_LDFLAGS) $(DEX_OBJECTS_math) $(LIBS) $(DEX_LIBS) $(LIBM)
endif


### net
ifndef CONFIG_WITHOUT_DEX_NET
DEX += net
DEX_SOURCES_net += $(wildcard $(SRC_ROOT)/src/dex/net/*.c)
DEX_OBJECTS_net = $(call dex_objects,net,$(DEX_SOURCES_net))
-include $(foreach F,$(DEX_OBJECTS_net),$(F:.o=.MF))
$(BLD_PATH)/dex/net/%.o: $(SRC_ROOT)/%.c
	@mkdir -p $(dir $@)
	$(DEX_CC) -MMD -MF $(@:.o=.MF) -c -o $@ $(CFLAGS) $(DEX_CFLAGS) $<
$(BIN_PATH)/lib/net$(DLL): $(DEX_OBJECTS_net) $(BIN_PATH)/deemon$(EXE)
	$(DEX_CC) -o $@ -L$(BIN_PATH) $(LDFLAGS) $(DEX_LDFLAGS) $(DEX_OBJECTS_net) $(LIBS) $(DEX_LIBS) $(LIBSOCKET)
endif


### posix
ifndef CONFIG_WITHOUT_DEX_POSIX
DEX += posix
DEX_SOURCES_posix += $(wildcard $(SRC_ROOT)/src/dex/posix/*.c)
DEX_OBJECTS_posix = $(call dex_objects,posix,$(DEX_SOURCES_posix))
-include $(foreach F,$(DEX_OBJECTS_posix),$(F:.o=.MF))
$(BLD_PATH)/dex/posix/%.o: $(SRC_ROOT)/%.c
	@mkdir -p $(dir $@)
	$(DEX_CC) -MMD -MF $(@:.o=.MF) -c -o $@ $(CFLAGS) $(DEX_CFLAGS) $<
$(BIN_PATH)/lib/posix$(DLL): $(DEX_OBJECTS_posix) $(BIN_PATH)/deemon$(EXE)
	$(DEX_CC) -o $@ -L$(BIN_PATH) $(LDFLAGS) $(DEX_LDFLAGS) $(DEX_OBJECTS_posix) $(LIBS) $(DEX_LIBS)
endif


### rt
DEX += rt
DEX_SOURCES_rt += $(wildcard $(SRC_ROOT)/src/dex/rt/*.c)
DEX_OBJECTS_rt = $(call dex_objects,rt,$(DEX_SOURCES_rt))
-include $(foreach F,$(DEX_OBJECTS_rt),$(F:.o=.MF))
$(BLD_PATH)/dex/rt/%.o: $(SRC_ROOT)/%.c
	@mkdir -p $(dir $@)
	$(DEX_CC) -MMD -MF $(@:.o=.MF) -c -o $@ $(CFLAGS) $(DEX_CFLAGS) $<
$(BIN_PATH)/lib/rt$(DLL): $(DEX_OBJECTS_rt) $(BIN_PATH)/deemon$(EXE)
	$(DEX_CC) -o $@ -L$(BIN_PATH) $(LDFLAGS) $(DEX_LDFLAGS) $(DEX_OBJECTS_rt) $(LIBS) $(DEX_LIBS)


### threading
ifndef CONFIG_WITHOUT_DEX_THREADING
DEX += threading
DEX_SOURCES_threading += $(wildcard $(SRC_ROOT)/src/dex/threading/*.c)
DEX_OBJECTS_threading = $(call dex_objects,threading,$(DEX_SOURCES_threading))
-include $(foreach F,$(DEX_OBJECTS_threading),$(F:.o=.MF))
$(BLD_PATH)/dex/threading/%.o: $(SRC_ROOT)/%.c
	@mkdir -p $(dir $@)
	$(DEX_CC) -MMD -MF $(@:.o=.MF) -c -o $@ $(CFLAGS) $(DEX_CFLAGS) $<
$(BIN_PATH)/lib/threading$(DLL): $(DEX_OBJECTS_threading) $(BIN_PATH)/deemon$(EXE)
	$(DEX_CC) -o $@ -L$(BIN_PATH) $(LDFLAGS) $(DEX_LDFLAGS) $(DEX_OBJECTS_threading) $(LIBS) $(DEX_LIBS)
endif


### time
ifndef CONFIG_WITHOUT_DEX_TIME
DEX += time
DEX_SOURCES_time += $(wildcard $(SRC_ROOT)/src/dex/time/*.c)
DEX_OBJECTS_time = $(call dex_objects,time,$(DEX_SOURCES_time))
-include $(foreach F,$(DEX_OBJECTS_time),$(F:.o=.MF))
$(BLD_PATH)/dex/time/%.o: $(SRC_ROOT)/%.c
	@mkdir -p $(dir $@)
	$(DEX_CC) -MMD -MF $(@:.o=.MF) -c -o $@ $(CFLAGS) $(DEX_CFLAGS) $<
$(BIN_PATH)/lib/time$(DLL): $(DEX_OBJECTS_time) $(BIN_PATH)/deemon$(EXE)
	$(DEX_CC) -o $@ -L$(BIN_PATH) $(LDFLAGS) $(DEX_LDFLAGS) $(DEX_OBJECTS_time) $(LIBS) $(DEX_LIBS)
endif


### win32
ifndef CONFIG_WITHOUT_DEX_WIN32
DEX += win32
DEX_SOURCES_win32 += $(wildcard $(SRC_ROOT)/src/dex/win32/*.c)
DEX_OBJECTS_win32 = $(call dex_objects,win32,$(DEX_SOURCES_win32))
-include $(foreach F,$(DEX_OBJECTS_win32),$(F:.o=.MF))
$(BLD_PATH)/dex/win32/%.o: $(SRC_ROOT)/%.c
	@mkdir -p $(dir $@)
	$(DEX_CC) -MMD -MF $(@:.o=.MF) -c -o $@ $(CFLAGS) $(DEX_CFLAGS) $<
$(BIN_PATH)/lib/win32$(DLL): $(DEX_OBJECTS_win32) $(BIN_PATH)/deemon$(EXE)
	$(DEX_CC) -o $@ -L$(BIN_PATH) $(LDFLAGS) $(DEX_LDFLAGS) $(DEX_OBJECTS_win32) $(LIBS) $(DEX_LIBS)
endif
