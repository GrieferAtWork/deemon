########################################################################
# Deemon Scripting Language Makefile
########################################################################

# https://stackoverflow.com/questions/18136918/how-to-get-current-relative-directory-of-your-makefile
MAKEFILE_PATH := $(shell dirname $(realpath $(lastword $(MAKEFILE_LIST))))

BLD_ROOT := $(MAKEFILE_PATH)
SRC_ROOT := $(MAKEFILE_PATH)

# Placeholder makefile configuration (these get overwritten by `config.mak')
CC := gcc
CC_DEX := g++
LDFLAGS :=
DLL := .dll
EXE := .exe
CFLAGS += -I$(SRC_ROOT)/include
CFLAGS += -I$(SRC_ROOT)/include/deemon/kos-headers
CFLAGS += -D__PE__
CORE_LDFLAGS += -Wl,--out-implib=$(BIN_PATH)/libdeemon.dll.a # windows-only
#CORE_LDFLAGS += -Wl,--stack,4194304 # i386
#CORE_LDFLAGS += -Wl,src/deemon/linker-scripts/link-deemon-gcc.def # i386
CORE_LDFLAGS += -Wl,--stack,8388608 # x86_64
CORE_LIBS +=
DEX_CFLAGS += -shared
DEX_LDFLAGS += -shared
DEX_LIBS +=
DEX_DEPENDENCIES +=

# Pull in the config file that gets created by ./configure
-include $(MAKEFILE_PATH)/config.mak

CORE_CFLAGS += -DCONFIG_BUILDING_DEEMON
DEX_CFLAGS += -DCONFIG_BUILDING_DEX
CFLAGS += -Wall -Wextra -Wno-address -Wno-unused-value -Wno-nonnull-compare -Wno-unused-parameter -Wno-comment -Wno-strict-aliasing -Wno-missing-field-initializers -Wno-type-limits -Wno-maybe-uninitialized

ifndef BIN_PATH
BIN_PATH := $(BLD_ROOT)
endif
ifndef BLD_PATH
BLD_PATH := $(BLD_ROOT)/build/deemon
endif
ifndef CC_DEX
CC_DEX := $(CC)
endif
ifndef CC_CORE
CC_CORE := $(CC)
endif




# From: https://stackoverflow.com/questions/3341482/in-a-makefile-how-to-get-the-relative-path-from-one-absolute-path-to-another
override define \s :=
$() $()
endef
ifndef $(\s)
override $(\s) :=
else
$(error Defined special variable '$(\s)': reserved for internal use)
endif
override define dirname
$(patsubst %/,%,$(dir $(patsubst %/,%,$1)))
endef
override define prefix_1
$(if $(or $\
$(patsubst $(abspath $3)%,,$(abspath $1)),$\
$(patsubst $(abspath $3)%,,$(abspath $2))),$\
$(strip $(call prefix_1,$1,$2,$(call dirname,$3))),$\
$(strip $(abspath $3)))
endef
override define prefix
$(call prefix_1,$1,$2,$1)
endef
override define relpath_1
$(patsubst /%,%,$(subst $(\s),/,$(patsubst %,..,$(subst /,$(\s),$\
$(patsubst $3%,%,$(abspath $2)))))$\
$(patsubst $3%,%,$(abspath $1)))
endef
override define relpath
$(call relpath_1,$1,$2,$(call prefix,$1,$2))
endef



# Include makfiles from the source folder
include $(SRC_ROOT)/src/deemon/Makefile
include $(SRC_ROOT)/src/dex/Makefile

# Tell Make how to build *.o files from *.c
$(BLD_PATH)/core/%.o: $(SRC_ROOT)/%.c
	@mkdir -p $(dir $@)
	$(CC_CORE) -MMD -MF $(@:.o=.MF) -c -o $@ $(CFLAGS) $(CORE_CFLAGS) $<

$(BIN_PATH)/deemon$(EXE): $(DEEMON_CORE_OBJECTS)
	$(CC_CORE) -o $@ $(LDFLAGS) $(CORE_LDFLAGS) $^ $(LIBS) $(CORE_LIBS)

.PHONY: all dex deemon
deemon: $(BIN_PATH)/deemon$(EXE)
dex: $(foreach F,$(DEX),$(BIN_PATH)/lib/$(F)$(DLL))
all: deemon dex

.DEFAULT_GOAL := all


