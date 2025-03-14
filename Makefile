########################################################################
# Deemon Scripting Language Makefile
########################################################################

# https://stackoverflow.com/questions/18136918/how-to-get-current-relative-directory-of-your-makefile
MAKEFILE_PATH := $(shell dirname $(realpath $(lastword $(MAKEFILE_LIST))))

BLD_ROOT := $(MAKEFILE_PATH)
SRC_ROOT := $(MAKEFILE_PATH)

# Pull in the config file that gets created by ./configure
include $(MAKEFILE_PATH)/config.mk

CFLAGS += -I$(SRC_ROOT)/include
CORE_CFLAGS += -DCONFIG_BUILDING_DEEMON
DEX_CFLAGS += -DCONFIG_BUILDING_DEX
CFLAGS += -Wall -Wextra -Wno-address -Wno-comment -Wno-array-bounds

ifndef BIN_PATH
BIN_PATH := $(BLD_ROOT)
endif
ifndef BLD_PATH
BLD_PATH := $(BLD_ROOT)/build/deemon
endif
ifndef DEX_CC
DEX_CC := $(CC)
endif
ifndef CORE_CC
CORE_CC := $(CC)
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
ifndef CONFIG_WITHOUT_DEX
include $(SRC_ROOT)/src/dex/Makefile
endif

# Tell Make how to build *.o files from *.c
$(BLD_PATH)/core/%.o: $(SRC_ROOT)/%.c
	@mkdir -p $(dir $@)
	$(CORE_CC) -MMD -MF $(@:.o=.MF) -c -o $@ $(CFLAGS) $(CORE_CFLAGS) $<
$(BLD_PATH)/core/%.o: $(SRC_ROOT)/%.S
	@mkdir -p $(dir $@)
	$(CORE_CC) -MMD -MF $(@:.o=.MF) -c -o $@ $(CFLAGS) $(CORE_CFLAGS) $<

$(BIN_PATH)/deemon$(EXE): $(DEEMON_CORE_OBJECTS)
	$(CORE_CC) -o $@ $(LDFLAGS) $(CORE_LDFLAGS) $^ $(LIBS) $(CORE_LIBS) $(LIBDL) || \
	$(CORE_CC) -o $@ $(LDFLAGS) $(CORE_LDFLAGS) $^ $(LIBS) $(CORE_LIBS) $(LIBDL) $(LIBM)

.PHONY: all deemon
deemon: $(BIN_PATH)/deemon$(EXE)

ifndef CONFIG_WITHOUT_DEX
.PHONY: dex
dex: $(foreach F,$(DEX),$(BIN_PATH)/lib/$(F)$(DLL))
all: deemon dex

.PHONY: dex.%
dex.%: $(BIN_PATH)/lib/%$(DLL)

else
all: deemon
endif

.DEFAULT_GOAL := all


# Misc script invocations

ifndef DEEMON
DEEMON := ./deemon
endif

vs-proj:
	$(DEEMON) util/make-vs-proj.dee
.PHONY: vs-proj

method-hints:
	$(DEEMON) -F \
		include/deemon/method-hints.h \
		include/deemon/operator-hints.h \
		src/deemon/objects/generic-proxy.h \
		src/deemon/objects/generic-proxy.c \
		src/deemon/objects/none.c \
		src/deemon/objects/object.c \
		src/deemon/runtime/method-hint-defaults.h \
		src/deemon/runtime/method-hint-defaults.c \
		src/deemon/runtime/method-hints.h \
		src/deemon/runtime/method-hints.c \
		src/deemon/runtime/method-hint-select.h \
		src/deemon/runtime/method-hint-select.c \
		src/deemon/runtime/method-hint-super.h \
		src/deemon/runtime/method-hint-super.c \
		src/deemon/runtime/method-hint-super-invoke.c \
		src/deemon/runtime/method-hint-wrappers.c \
		src/deemon/runtime/operator-hints.c \
		src/deemon/runtime/operator-hint-defaults.c \
		src/deemon/runtime/operator-hint-errors.h \
		src/deemon/runtime/operator-hint-errors.c \
		src/deemon/runtime/operator-hint-invoke.c \
		src/deemon/runtime/strings.h \
		lib/rt/hints/method.dee \
		lib/rt/hints/operator.dee
.PHONY: method-hints

computed-operators:
	$(DEEMON) util/scripts/computed-operators.dee
.PHONY: computed-operators
