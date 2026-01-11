########################################################################
# Deemon Scripting Language Makefile
########################################################################

.DELETE_ON_ERROR:

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
$(or $(call relpath_1,$1,$2,$(call prefix,$1,$2)),.)
endef
define LF


endef

# $(call if_list_longer_than,list,N,YES,NO)
# Expands to $(YES) if $(words $(list)) is greater-or-equal >= $(N); else expands to $(NO)
define if_list_longer_than
$(if $(wordlist $(2),$(words $(1)),$(1)),$(3),$(4))
endef

# https://stackoverflow.com/questions/18136918/how-to-get-current-relative-directory-of-your-makefile
PWD := $(shell pwd)
MAKEFILE_PATH := $(call dirname $(realpath $(lastword $(MAKEFILE_LIST))))
MAKEFILE_RELPATH := $(call relpath,$(MAKEFILE_PATH),$(PWD))
MAKECMDGOALS_RELPATH := $(foreach g,$(MAKECMDGOALS),$(call relpath,$(g),$(PWD)))



# Pull in the config file that gets created by ./configure
include $(PWD)/config.mk


ifndef SRC_ROOT
SRC_ROOT := $(MAKEFILE_PATH)
endif
ifndef BLD_ROOT
BLD_ROOT := $(MAKEFILE_PATH)/build/deemon
endif
ifndef BIN_ROOT
BIN_ROOT := $(MAKEFILE_PATH)
endif
ifndef DEX_CC
DEX_CC := $(CC)
endif
ifndef CORE_CC
CORE_CC := $(CC)
endif
SRC_RELPATH := $(call relpath,$(SRC_ROOT),$(PWD))
BLD_RELPATH := $(call relpath,$(BLD_ROOT),$(PWD))
BIN_RELPATH := $(call relpath,$(BIN_ROOT),$(PWD))




## =======================================================================
## =======================================================================
## =======================================================================
##
## BUILD CONFIGURATION
##
## =======================================================================
## =======================================================================
## =======================================================================


# Compiler flags configuration for *all* source files
CFLAGS_src := $(CFLAGS)
CFLAGS_src += -I$(call relpath,$(SRC_ROOT)/include,$(PWD))
CFLAGS_src += -Wall -Wextra -Wno-address -Wno-comment -Wno-array-bounds

# Deemon core
CC_src_deemon     := $(CORE_CC) # Everything in /src/deemon
CFLAGS_src_deemon := $(CORE_CFLAGS) -DCONFIG_BUILDING_DEEMON
LD_deemon$(EXE)   := $(CC_src_deemon)
SRC_deemon$(EXE) := \
	src/deemon/*.c \
	src/deemon/compiler/*.c \
	src/deemon/compiler/asm/*.c \
	src/deemon/compiler/lexer/*.c \
	src/deemon/compiler/interface/*.c \
	src/deemon/compiler/optimize/*.c \
	src/deemon/execute/*.c \
	src/deemon/system/*.c \
	src/deemon/objects/*.c \
	src/deemon/objects/seq/*.c \
	src/deemon/objects/unicode/*.c \
	src/deemon/runtime/*.c
# Special rule for building "exec.c" (try to embed a build timestamp within that file)
ifndef CONFIG_SH_NO_DATE_TIMESTAMP
CFLAGS_src_deemon_execute_exec.c = -DBUILD_TIMESTAMP=$(shell date -u +%s)
endif
LDFLAGS_deemon$(EXE) := $(LDFLAGS) $(CORE_LDFLAGS)
LIBS_deemon$(EXE)    := $(CORE_LIBS) $(LIBDL)
OPTLIBS_deemon$(EXE) := $(LIBM)         # Optional library dependency (if link fails w/o, try with)
BIN_MANDATORY += deemon$(EXE)           # The deemon core is a mandatory binary


# Dex modules
CC_src_dex  := $(DEX_CC)                   # Everything in /src/dex
CFLAGS_src_dex += $(DEX_CFLAGS) -DCONFIG_BUILDING_DEX
LD_lib      := $(CC_src_dex)               # Default linker is for dex modules
LDFLAGS_lib := $(LDFLAGS) $(DEX_LDFLAGS)   # Linker flags for dex modules
LIBS_lib    := $(DEX_LIBS)                 # Lib dependencies for dex modules
LIBDEPS_lib := $(BIN_RELPATH)/deemon$(EXE) # Files to built before building /lib  (iow: build deemon core before dex modules)


# Dex modules...
#BIN_OPTIONAL += lib/_hostasm$(DLL)
#SRC_lib__hostasm$(DLL) := src/dex/_hostasm/*.c


BIN_OPTIONAL += lib/_strexec$(DLL)
SRC_lib__strexec$(DLL) := src/dex/_strexec/*.c


BIN_OPTIONAL += lib/collections$(DLL)
SRC_lib_collections$(DLL) := src/dex/collections/*.c


ifndef CONFIG_WITHOUT_DEX_CTYPES
BIN_OPTIONAL += lib/ctypes$(DLL)
ifdef DEX_CTYPES_CC
CC_src_dex_ctypes := $(DEX_CTYPES_CC)
else
CC_src_dex_ctypes := $(DEX_CC)
endif
CFLAGS_src_dex_ctypes := $(DEX_CTYPES_CFLAGS)
LD_lib_ctypes$(DLL) := $(CC_src_dex)
LDFLAGS_lib_ctypes$(DLL) := $(DEX_CTYPES_LDFLAGS)
LIBS_lib_ctypes$(DLL) := $(DEX_CTYPES_LIBS)
SRC_lib_ctypes$(DLL) := src/dex/ctypes/*.c
endif


BIN_OPTIONAL += lib/disassembler$(DLL)
SRC_lib_disassembler$(DLL) := src/dex/disassembler/*.c


BIN_OPTIONAL += lib/files$(DLL)
SRC_lib_files$(DLL) := src/dex/files/*.c


BIN_OPTIONAL += lib/hashlib$(DLL)
SRC_lib_hashlib$(DLL) := src/dex/hashlib/*.c


ifndef CONFIG_WITHOUT_DEX_IPC
BIN_OPTIONAL += lib/ipc$(DLL)
SRC_lib_ipc$(DLL) := src/dex/ipc/*.c
endif


ifndef CONFIG_WITHOUT_DEX_JSON
BIN_OPTIONAL += lib/json$(DLL)
SRC_lib_json$(DLL) := src/dex/json/*.c
endif


ifndef CONFIG_WITHOUT_DEX_MATH
BIN_OPTIONAL += lib/math$(DLL)
SRC_lib_math$(DLL) := src/dex/math/*.c
OPTLIBS_lib_math$(DLL) := $(LIBM)
endif


ifndef CONFIG_WITHOUT_DEX_NET
BIN_OPTIONAL += lib/net$(DLL)
SRC_lib_net$(DLL) := src/dex/net/*.c
LIBS_lib_net$(DLL) := $(LIBSOCKET)
endif


ifndef CONFIG_WITHOUT_DEX_POSIX
BIN_OPTIONAL += lib/posix$(DLL)
SRC_lib_posix$(DLL) := src/dex/posix/*.c
endif


BIN_OPTIONAL += lib/rt$(DLL)
SRC_lib_rt$(DLL) := src/dex/rt/*.c


ifndef CONFIG_WITHOUT_DEX_SQLITE3
BIN_OPTIONAL += lib/sqlite3$(DLL)
SRC_lib_sqlite3$(DLL) := src/dex/sqlite3/*.c
# Special case: compile "sqlite3-external.c" as C-code (and never as C++ code)
CC_src_dex_sqlite3_sqlite3-external.c := $(CC)
# Include $(LIBDL) here because sqlite needs it for extensions
LIBS_lib_sqlite3$(DLL) := $(LIBDL)
endif


ifndef CONFIG_WITHOUT_DEX_THREADING
BIN_OPTIONAL += lib/threading$(DLL)
SRC_lib_threading$(DLL) := src/dex/threading/*.c
endif


ifndef CONFIG_WITHOUT_DEX_TIME
BIN_OPTIONAL += lib/time$(DLL)
SRC_lib_time$(DLL) := src/dex/time/*.c
endif


ifndef CONFIG_WITHOUT_DEX_WIN32
BIN_OPTIONAL += lib/win32$(DLL)
SRC_lib_win32$(DLL) := src/dex/win32/*.c
endif







## =======================================================================
## Compiler machinery
## =======================================================================
override define canon
$(subst /,_,$(1))
endef
override define canon_all_asc
$(call canon,$(1)) \
$(if $(findstring /,$(1)),$(call canon_all_asc,$(patsubst %/,%,$(dir $(1)))))
endef
override define canon_all_dsc
$(if $(findstring /,$(1)),$(call canon_all_dsc,$(patsubst %/,%,$(dir $(1))))) \
$(call canon,$(1))
endef
override define src_to_obj
$(BLD_RELPATH)/$(patsubst %.c,%.o,$(1))
endef
override define src_to_dep
$(BLD_RELPATH)/$(patsubst %.c,%.MF,$(1))
endef
override define bin_sources
$(foreach p,$(SRC_$(call canon,$(1))),$(foreach f,$(wildcard $(SRC_ROOT)/$(p)),$(call relpath,$(f),$(SRC_ROOT))))
endef
override define bin_objects
$(foreach s,$(call bin_sources,$(1)),$(call src_to_obj,$(s)))
endef
override define bin_deps
$(foreach s,$(call bin_sources,$(1)),$(call src_to_dep,$(s)))
endef

OBJ_OPTIONAL  := $(foreach lib,$(BIN_OPTIONAL),$(call bin_objects,$(lib)))
OBJ_MANDATORY := $(foreach lib,$(BIN_MANDATORY),$(call bin_objects,$(lib)))
OBJ_ALL := $(OBJ_OPTIONAL) $(OBJ_MANDATORY)
_BIN_ALL := $(BIN_MANDATORY) $(BIN_OPTIONAL)            # Raw filenames
BIN_ALL := $(foreach b,$(_BIN_ALL),$(BIN_RELPATH)/$(b)) # Relative (resolvable) filenames
DRYRUN := $(findstring n,$(MAKEFLAGS))


## =======================================================================
## General purpose C/C++ compiler rule
## =======================================================================
##
## CC_{canon_names_asc}      := gcc       (Compiler to use)
## CFLAGS_{canon_names_dsc}  := -g -O3    (Compiler flags)
##
## =======================================================================
$(BLD_ROOT)/%.o $(BLD_RELPATH)/%.o: $(SRC_RELPATH)/%.c
# make -f deemon/Makefile -B -n deemon/build/deemon/src/deemon/cmdline.o
#	canon_src=src/deemon/cmdline.c
#	canon_names_asc=src_deemon_cmdline.c src_deemon src
#	canon_names_dsc=src src_deemon src_deemon_cmdline.c
#	src=deemon/src/deemon/cmdline.c
#	obj=deemon/build/deemon/src/deemon/cmdline.o
#	dep=deemon/build/deemon/src/deemon/cmdline.MF
	$(eval canon_src := $(call relpath,$<,$(SRC_ROOT)))
	$(eval canon_names_asc := $(call canon_all_asc,$(canon_src)))
	$(eval canon_names_dsc := $(call canon_all_dsc,$(canon_src)))
	$(eval src := $(call relpath,$<,$(PWD)))
	$(eval obj := $(call relpath,$@,$(PWD)))
	$(eval dep := $(patsubst %.o,%.MF,$(obj)))
#	cc: Compiler to use for this file: First one of:
#		- CC_src_deemon_cmdline_c
#		- CC_src_deemon
#		- CC_src
#		- CC
	$(eval cc := $(or $(firstword $(foreach c,$(canon_names_asc),$(CC_$(c)))),$(CC)))
#	cflags: Compiler flags for this file: union one of:
#		- CFLAGS_src_deemon_cmdline_c
#		- CFLAGS_src_deemon
#		- CFLAGS_src                     (this one gets set to $(CFLAGS) above)
	$(eval cflags := $(foreach c,$(canon_names_dsc),$(CFLAGS_$(c))))
	$(eval cflags := $(filter-out ,$(cflags)))
#	echo canon_src=$(canon_src)
#	echo canon_names_asc=$(canon_names_asc)
#	echo canon_names_dsc=$(canon_names_dsc)
#	echo cc=$(cc)
#	echo cflags=$(cflags)
#	echo src=$(src)
#	echo obj=$(obj)
#	echo dep=$(dep)
ifeq (,$(DRYRUN))
	$(eval is_mandatory := $(or $(if $(filter $(obj),$(OBJ_OPTIONAL)),,y),$(filter $@,$(MAKECMDGOALS))))
	$(eval cmd_suffix := $(if $(is_mandatory),, || { \
		rm -f $(obj) $(dep); \
		echo "[33m⚠️[m  Failed to compile optional source file [97m$<[m"; \
	}))
	@echo "$(if $(is_mandatory),,[94mopt[m )$(cc) -c $(filter-out $(CFLAGS_src),$(cflags)) $(src)"
	@mkdir -p $(dir $@)
	@$(cc) -c -o $(obj) -MMD -MP -MF $(dep) $(cflags) $(src) $(cmd_suffix)
else
	$(cc) -c -o $(obj) -MMD -MP -MF $(dep) $(cflags) $(src)
endif




## =======================================================================
## Rule creation macro for linking object files into a binary
## =======================================================================
##
## LD_{canon_names_asc}      := ld                           (Linker program to use)
## LDFLAGS_{canon_names_dsc} := -shared -L.                  (Extra linker flags)
## LIBS_{canon_names_dsc}    := -ldeemon                     (Extra libraries to link against)
## OPTLIBS_{canon_names_dsc} := -lm                          (Optional libraries; try link w/o. If that fails, try link w/)
## LIBDEPS_{canon_names_dsc} := $(BIN_RELPATH)/deemon.exe    (Other binary files to build first; artificial dependencies)
##
## =======================================================================
define LINK_RECIPE
#canon_name=lib_rt.dll
#canon_names_asc=lib_rt.dll lib
#canon_names_dsc=lib lib_rt.dll
$(eval canon_name := $(call canon,$(1)))
$(eval canon_names_asc := $(call canon_all_asc,$(1)))
$(eval canon_names_dsc := $(call canon_all_dsc,$(1)))
#ld=g++
#ldflags=-shared -L.
#libs=-ldeemon
#optlibs=-lm
$(eval ld := $(or $(firstword $(foreach c,$(canon_names_asc),$(LD_$(c)))),$(LD)))
$(eval ldflags := $(foreach c,$(canon_names_dsc),$(LDFLAGS_$(c))))
$(eval libs := $(foreach c,$(canon_names_dsc),$(LIBS_$(c))))
$(eval optlibs := $(foreach c,$(canon_names_dsc),$(OPTLIBS_$(c))))
$(eval is_mandatory := $(or $(if $(filter $(1),$(BIN_OPTIONAL)),,y),$(filter $(call relpath,$(1),$(PWD)),$(MAKECMDGOALS_RELPATH))))
$(eval cli_objs_limit := $(if $(optlibs),128,256))

# The actual library building rule...
$(BIN_ROOT)/$(1) $(BIN_RELPATH)/$(1): $(call bin_objects,$(1)) $(foreach c,$(canon_names_dsc),$(LIBDEPS_$(c)))
#	bin=lib/rt.dll
#	objs=build/deemon/src/dex/rt/librt.o build/deemon/src/dex/rt/slab.o build/deemon/src/dex/rt/string-fini-hook.o
#	ld=g++
	$$(eval bin := $$(call relpath,$$@,$$(PWD)))
	$$(eval objs := $$(foreach o,$$(call bin_objects,$(1)),$$(call relpath,$$(o),$$(PWD))))

# Automatically dump object files in a CLI file if the commandline would become too large, otherwise
	$$(eval maybe_export_objects1 := $$(call if_list_longer_than,$$(objs),$(cli_objs_limit), \
		mkdir -p $$(BLD_RELPATH) \
	,))
# TODO: Don't use printf here to print everything all at once -- the whole point of dumping
#       object file names into a separate file is to work around cmdline length limits, and
#       if we just repeat all object names on a singular printf-call, we just hit the same
#       limit but with "printf" instead of "gcc"!
	$$(eval maybe_export_objects2 := $$(if $$(maybe_export_objects1), \
		printf "$$(foreach o,$$(objs),$$(subst \\,\\\\,$$(o))\\n)" > $$(BLD_RELPATH)/$(1).args \
	,))
	$$(eval objs := $$(if $$(maybe_export_objects1), @$$(BLD_RELPATH)/$(1).args, $$(objs)))
ifeq (,$(DRYRUN))
	@$$(maybe_export_objects1)
endif
	@$$(maybe_export_objects2)

# Build the linker CLI that will need to be called
	$$(eval link_cmd_base := $(ld) $(ldflags) -o $$(bin) $$(objs) $(libs))
	$$(eval link_cmd := $$(if $(optlibs),$$(link_cmd_base) || $$(link_cmd_base) $(optlibs),$$(link_cmd_base)))
	$$(eval link_cmd := $$(filter-out ,$$(link_cmd)))

#	echo bin=$$(bin)
#	echo objs=$$(objs)
#	echo canon_name=$(canon_name)
#	echo canon_names_asc=$(canon_names_asc)
#	echo canon_names_dsc=$(canon_names_dsc)
#	echo ld=$(ld)
#	echo ldflags=$(ldflags)
#	echo libs=$(libs)
#	echo optlibs=$(optlibs)

ifeq (,$(DRYRUN))
	@mkdir -p $$(dir $$(bin))
ifeq (,$(is_mandatory))
	$$(eval present_objs := $$(foreach o,$$(objs),$$(wildcard $$(o))))
	$$(eval missing_objs := $$(sort $$(filter-out $$(present_objs),$$(objs))))
	$$(eval final_link_cmd := $$(if $$(missing_objs), \
		echo "[33m⚠️[m  Missing [97m$$(firstword $$(missing_objs))[m$$(if $$(word 2,$$(missing_objs)), and $$(words $$(filter-out $$(firstword $$(missing_objs)),$$(missing_objs))) more,): skipping [97m$$@[m (see log for details)"; \
		rm -f $$(bin); \
	, \
		echo "[94mopt[m $(ld) $(ldflags) -o $$(bin) ... $(libs)"; \
		$$(link_cmd) || rm -f $$(bin) \
	))
	@$$(final_link_cmd)
else
	@echo $(ld) $(ldflags) -o $$(bin) ... $(libs)
	@$$(link_cmd)
endif
else
	$$(link_cmd)
endif
endef

# Generate rules for building binaries (both mandatory-, and optional ones)
$(foreach lib,$(_BIN_ALL),$(eval $(call LINK_RECIPE,$(lib))))

# Auto-include dependency files
-include $(OBJ_ALL:.o=.MF)


## =======================================================================
## The big, magical "make all" rule
## =======================================================================
.PHONY: all _print_optional_module_summary
all: $(BIN_ALL) _print_optional_module_summary
.DEFAULT_GOAL := all

## =======================================================================
## Summary of failed, optional binaries
## =======================================================================
ifeq (,$(DRYRUN))
_print_optional_module_summary: $(BIN_ALL)
	$(eval bins := $(foreach b,$(BIN_ALL),$(call relpath,$(b),$(PWD))))
	$(eval present_bins := $(foreach b,$(bins),$(wildcard $(b))))
	$(eval missing_bins := $(sort $(filter-out $(present_bins),$(bins))))
	$(eval body := $(if $(missing_bins), \
		echo ""; \
		echo ""; \
		echo "Some binaries could not be built (see log for details):"; \
		$(foreach b,$(present_bins), echo "[92m✅[m  Built $(b)";) \
		$(foreach b,$(missing_bins), echo "[91m❌[m  Failed $(b)";) \
	, \
		true \
	))
	@$(body)
else
_print_optional_module_summary:
endif


## =======================================================================
## Misc. convenience make commands
## =======================================================================
clean: rmdec
	rm -rf $(BLD_RELPATH)/src
	rm -f $(BLD_RELPATH)/deemon.exe.args $(SRC_RELPATH)/LastCoverageResults.log $(BIN_RELPATH)/libdeemon.dll.a
	rm -f $(BIN_ALL)
	rm -f $(BIN_ALL:$(EXE)=.exp)
	rm -f $(BIN_ALL:$(EXE)=.lib)
	rm -f $(BIN_ALL:$(EXE)=.pdb)
	rm -f $(BIN_ALL:$(DLL)=.exp)
	rm -f $(BIN_ALL:$(DLL)=.lib)
	rm -f $(BIN_ALL:$(DLL)=.pdb)

install:
	$(eval destdir := $(or $(DESTDIR),$(PWD)))
	$(eval abs_destdir := $(realpath $(destdir)))
	$(eval install_src_files := $(wildcard $(SRC_RELPATH)/lib/***.dee))
	$(eval install_src_files += $(wildcard $(SRC_RELPATH)/lib/**/*.dee))
	$(eval install_src_files += $(wildcard $(SRC_RELPATH)/lib/**/**/*.dee))
	$(eval install_src_files += $(wildcard $(SRC_RELPATH)/lib/**/**/**/*.dee))
	$(eval install_src_files += $(wildcard $(SRC_RELPATH)/lib/**/**/**/**/*.dee))
	$(eval install_src_files += $(wildcard $(SRC_RELPATH)/lib/include/*))
	$(eval install_src_files += $(wildcard $(SRC_RELPATH)/lib/include/**/*))
	$(eval install_src_files += $(wildcard $(SRC_RELPATH)/lib/include/**/**/*))
	$(eval install_src_files += $(wildcard $(SRC_RELPATH)/lib/include/**/**/**/*))
	$(eval install_src_files := $(foreach f,$(install_src_files),$(call relpath,$(f),$(SRC_RELPATH))))
	$(eval install_src_files := $(filter-out $(foreach f,$(install_src_files),$(patsubst %/,%,$(dir $(f)))),$(install_src_files)))
	$(eval install_src_files := $(filter-out %.dec,$(install_src_files)))
	$(eval install_src_files := $(filter-out %.txt,$(install_src_files)))
	$(eval install_src_files := $(sort $(install_src_files)))
	$(eval install_bin_files := $(foreach b,$(BIN_ALL),$(wildcard $(b))))
	$(eval install_bin_files := $(foreach f,$(install_bin_files),$(call relpath,$(f),$(BIN_RELPATH))))
	$(eval install_bin_files := $(filter-out $(foreach f,$(install_bin_files),$(patsubst %/,%,$(dir $(f)))),$(install_bin_files)))
	$(eval install_bin_files := $(sort $(install_bin_files)))
	$(eval install_dirs := $(filter-out ./,$(sort $(foreach f,$(install_src_files) $(install_bin_files),$(dir $(f))))))
#	$(foreach d,$(install_dirs),mkdir -p "$(destdir)/$(d)"$(LF))
	$(foreach d,$(install_dirs),$(if $(wildcard $(destdir)/$(d)),,mkdir -p "$(destdir)/$(d)"$(LF)))
	$(if $(filter-out $(abs_destdir),$(BIN_ROOT)), \
		$(foreach d,$(install_bin_files),cp "$(BIN_RELPATH)/$(d)" "$(destdir)/$(d)"$(LF)) \
	,)
	$(if $(filter-out $(abs_destdir),$(SRC_ROOT)), \
		$(foreach d,$(install_src_files),cp "$(SRC_RELPATH)/$(d)" "$(destdir)/$(d)" $(LF)) \
	,)
	@echo "\
If you want [97mdeemon[m to be part of you \$$PATH, run:\n\
[97msudo ln -s "$(abs_destdir)/deemon" /usr/bin/deemon[m"






## =======================================================================
## Misc script invocations
## =======================================================================

help:
	@printf "\
Build commands\n\
\t[97mmake[m                     Same as [97mmake all[m\n\
\t[97mmake all[m                 Build deemon and (try to) build dex modules. Failure\n\
\t[97m[m                         to build dex modules does not constitute an error, but\n\
\t[97m[m                         all binaries that could not be built are summarized at\n\
\t[97m[m                         the end.\n\
\t[97mmake FILE[m                Build a specific file. Failure while building any of\n\
\t[97m[m                         these files constitutes an error\n\
\t[97mmake install [DESTDIR=.][m Install binaries and modules to DESTDIR. Unless\n\
\t[97m[m                         [97mCONFIG_DEEMON_HOME[m was used, deemon doesn't need to\n\
\t[97m[m                         know its install location during building. If you want\n\
\t[97m[m                         to have deemon as part of your \$$PATH, use\n\
\t[97m[m                         [97msudo ln -s \$$DESTDIR/deemon /usr/bin/deemon[m\n\
\t[97mmake clean[m               Delete all object files/binaries (includes [97mrmdec[m)\n\
\n\
Maintainer commands\n\
\t[97mmake vs-proj[m             Generate Visual Studio project files ([97m.vs/deemon-vNNN.sln[m)\n\
\t[97mmake method-hints[m        Re-build generated source code for method hints\n\
\t[97mmake computed-operators[m  Re-build generated source code for computed operators\n\
\t[97m[m                         Set [97mCONFIG_WITHOUT_COMPUTED_DEFAULT_OPERATORS[m first\n\
\t[97mmake cxx-generated[m       Re-build generated source code for c++ headers\n\
\t[97mmake rmdec[m               Removed pre-compiled deemon modules ([97m*.dec[m files)\n\
\n\
Special care has been taken so [97mmake -j[m and [97mmake -n[m work as intended for all make commands"
.PHONY: help


ifndef DEEMON
DEEMON := $(BIN_RELPATH)/deemon$(EXE)
endif

rmdec:
	find -name '*.dec' -delete
.PHONY: rmdec

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

cxx-generated:
	$(DEEMON) -F -Wno-user -Wno-usage \
		include/deemon/cxx/bool.h \
		include/deemon/cxx/bytes.h \
		include/deemon/cxx/callable.h \
		include/deemon/cxx/cell.h \
		include/deemon/cxx/dict.h \
		include/deemon/cxx/file.h \
		include/deemon/cxx/float.h \
		include/deemon/cxx/function.h \
		include/deemon/cxx/hashset.h \
		include/deemon/cxx/int.h \
		include/deemon/cxx/iterator.h \
		include/deemon/cxx/list.h \
		include/deemon/cxx/mapping.h \
		include/deemon/cxx/none.h \
		include/deemon/cxx/numeric.h \
		include/deemon/cxx/object.h \
		include/deemon/cxx/sequence.h \
		include/deemon/cxx/set.h \
		include/deemon/cxx/string.h \
		include/deemon/cxx/tuple.h \
		include/deemon/cxx/type.h
.PHONY: cxx-generated
