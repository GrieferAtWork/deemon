########################################################################
# DEEMON CORE
########################################################################

DEEMON_CORE_SOURCE_FILES += $(wildcard $(SRC_ROOT)/src/deemon/*.c)
DEEMON_CORE_SOURCE_FILES += $(wildcard $(SRC_ROOT)/src/deemon/compiler/*.c)
DEEMON_CORE_SOURCE_FILES += $(wildcard $(SRC_ROOT)/src/deemon/compiler/asm/*.c)
DEEMON_CORE_SOURCE_FILES += $(wildcard $(SRC_ROOT)/src/deemon/compiler/lexer/*.c)
DEEMON_CORE_SOURCE_FILES += $(wildcard $(SRC_ROOT)/src/deemon/compiler/interface/*.c)
DEEMON_CORE_SOURCE_FILES += $(wildcard $(SRC_ROOT)/src/deemon/compiler/optimize/*.c)
DEEMON_CORE_SOURCE_FILES += $(wildcard $(SRC_ROOT)/src/deemon/execute/*.c)
DEEMON_CORE_SOURCE_FILES += $(wildcard $(SRC_ROOT)/src/deemon/system/*.c)
DEEMON_CORE_SOURCE_FILES += $(wildcard $(SRC_ROOT)/src/deemon/objects/*.c)
DEEMON_CORE_SOURCE_FILES += $(wildcard $(SRC_ROOT)/src/deemon/objects/seq/*.c)
DEEMON_CORE_SOURCE_FILES += $(wildcard $(SRC_ROOT)/src/deemon/objects/unicode/*.c)
DEEMON_CORE_SOURCE_FILES += $(wildcard $(SRC_ROOT)/src/deemon/runtime/*.c)

DEEMON_CORE_OBJECTS = $(foreach F,$(DEEMON_CORE_SOURCE_FILES),$(BLD_PATH)/core/$(call relpath,$(F:.c=.o),$(SRC_ROOT)))

# Include GCC's automatically generated source dependency files
-include $(foreach F,$(DEEMON_CORE_OBJECTS),$(F:.o=.MF))

