# ==========================================
#   Unity Project - A Test Framework for C
#   Copyright (c) 2007 Mike Karlesky, Mark VanderVoord, Greg Williams
#   [Released under MIT License. Please refer to license.txt for details]
# ==========================================

group?=
CFLAGS?=

#We try to detect the OS we are running on, and adjust commands as needed
ifeq ($(OSTYPE),cygwin)
	CLEANUP          = rm -f
	MKDIR            = mkdir -p
	TARGET_EXTENSION =.out
elseifeq ($(OSTYPE),msys)
	CLEANUP          = rm -f
	MKDIR            = mkdir -p
	TARGET_EXTENSION =.exe
elseifeq ($(OS),Windows_NT)
	CLEANUP          = del /F /Q
	MKDIR            = mkdir
	TARGET_EXTENSION =.exe
else
	CLEANUP          = rm -f
	MKDIR            = mkdir -p
	TARGET_EXTENSION =.out
endif

dir_guard=@mkdir -p $(@D)

PATH_TOP   = ./
PATH_SRC   = $(PATH_TOP)src/
PATH_LIB   = $(PATH_TOP)lib/
PATH_BUILD = $(PATH_TOP)build/

PATH_CBOR  = $(PATH_SRC)
INC_DIRS  += -I $(PATH_CBOR)
HDR_ALL   += $(wildcard $(PATH_CBOR)*.h)
SRC_CBOR   = $(wildcard $(PATH_CBOR)*.c)
SRC_ALL   += $(SRC_CBOR)
OBJ_CBOR   = $(patsubst $(PATH_CBOR)%.c,$(PATH_BUILD)%.o,$(SRC_CBOR))
OBJ_ALL   += $(OBJ_CBOR)

PATH_UNITY  = $(PATH_LIB)unity/src/
INC_DIRS   += -I $(PATH_UNITY)
HDR_ALL    += $(wildcard $(PATH_UNITY)*.h)
SRC_UNITY   = $(wildcard $(PATH_UNITY)*.c)
SRC_ALL    += $(SRC_UNITY)
OBJ_UNITY   = $(patsubst $(PATH_UNITY)%.c,$(PATH_BUILD)%.o,$(SRC_UNITY))
OBJ_ALL    += $(OBJ_UNITY)

PATH_UNITY_FIXT  = $(PATH_LIB)unity/extras/fixture/src/
INC_DIRS        += -I $(PATH_UNITY_FIXT)
HDR_ALL         += $(wildcard $(PATH_UNITY_FIXT)*.h)
SRC_UNITY_FIXT   = $(wildcard $(PATH_UNITY_FIXT)*.c)
SRC_ALL         += $(SRC_UNITY_FIXT)
OBJ_UNITY_FIXT   = $(patsubst $(PATH_UNITY_FIXT)%.c,$(PATH_BUILD)%.o,$(SRC_UNITY_FIXT))
OBJ_ALL         += $(OBJ_UNITY_FIXT)

PATH_TEST  = $(PATH_TOP)test/
INC_DIRS  += -I $(PATH_TEST)
HDR_ALL    += $(wildcard $(PATH_UNITY)*.h)
SRC_TEST   = $(wildcard $(PATH_TEST)*.c)
SRC_ALL   += $(SRC_TEST)
OBJ_TEST   = $(patsubst $(PATH_TEST)%.c,$(PATH_BUILD)%.o,$(SRC_TEST))
OBJ_ALL   += $(OBJ_TEST)

CHECK_SRC += $(filter $(PATH_SRC)% $(PATH_TEST)%,$(SRC_ALL))
CHECK_SRC += $(filter $(PATH_SRC)% $(PATH_TEST)%,$(HDR_ALL))

TGT     = $(PATH_BUILD)test$(TARGET_EXTENSION)
RESULTS = $(PATH_BUILD)results.txt

#Tool Definitions
C_COMPILER = clang
CFLAGS    += -std=c99
CFLAGS    += -D UNIT_TEST
CFLAGS    += -g
CFLAGS    += -O0
CFLAGS    += -Wall
CFLAGS    += -ferror-limit=5

OVERRRIDES += -include "unity_fixture_malloc_overrides.h"
OVERRRIDES += -include "assert_override.h"
COV_FLAGS  += -fprofile-instr-generate
COV_FLAGS  += -fcoverage-mapping

RUN_FLAGS=-g "$(group)"
ifeq ($(group),)
	COV_REP_FLAGS=$(TGT)
else
	COV_REP_FLAGS=$(PATH_BUILD)*$(group)*.o
endif

COMPILE     = $(C_COMPILER) -c $(CFLAGS) $(INC_DIRS) $< -o $@
COMPILE_OV  = $(C_COMPILER) -c $(CFLAGS) $(OVERRRIDES) $(INC_DIRS) $< -o $@
COMPILE_COV = $(C_COMPILER) -c $(CFLAGS) $(OVERRRIDES) $(COV_FLAGS)  $(INC_DIRS) $< -o $@
LINK        = $(C_COMPILER) $(COV_FLAGS) -o $@ $^

#Result formatting
NO_COLOR = sgr0
GREEN    = setaf 2
RED      = setaf 1
YELLOW   = setaf 3

PASSED_TESTS = echo; \
	echo ----Passed--------------------------; \
	grep -s -E '.*PASS' ./$(RESULTS); true

IGNORED_TESTS = echo; \
	echo ----Ignored-------------------------; \
	grep -s -E '.*IGNORE' ./$(RESULTS); true

FAILED_TESTS = echo; \
	echo ----Failed--------------------------; \
	grep -s -E '.*FAIL' ./$(RESULTS); true

RESULT_SUMMARY = echo; \
	echo -----------------------------------; \
	grep -s Ignored ./$(RESULTS)

FINAL_RESULT = if grep -q FAIL ./$(RESULTS); \
	then tput $(RED);   echo ==============FAIL==============; tput $(NO_COLOR); \
	else tput $(GREEN); echo ==============PASS==============; tput $(NO_COLOR); \
	fi

default: cov-summary
	@$(IGNORED_TESTS)
	@$(FAILED_TESTS)
	@$(RESULT_SUMMARY)
	@$(FINAL_RESULT)

check:
	@echo ----CPPCHECK-----------------------------
	@cppcheck --enable=all --check-config --suppress=missingIncludeSystem      \
		$(INC_DIRS) $(CHECK_SRC) >/dev/null
	@echo ----CLANG-TIDY---------------------------
	@clang-tidy $(CHECK_SRC)                  \
		2>/dev/null

clean:
	@$(CLEANUP) $(PATH_BUILD)*.o
	@$(CLEANUP) $(TGT)

clean-all:
	@$(CLEANUP) -r $(PATH_LIB)
	@$(CLEANUP) -r $(PATH_BUILD)

clean-lib:
	@$(CLEANUP) -r $(PATH_LIB)

cov-summary: $(PATH_BUILD)default.profdata
	@llvm-cov report -instr-profile=$(PATH_BUILD)default.profdata \
		$(COV_REP_FLAGS)

coverage: $(PATH_BUILD)default.profdata
	@llvm-cov show  -instr-profile=$(PATH_BUILD)default.profdata \
		-line-coverage-lt=100 $(COV_REP_FLAGS)

debug: $(PATH_BUILD) $(TGT)
	@gdb -ex 'b assert_override.c:13' ./$(TGT)

docs:
	@$(MKDIR) $(PATH_BUILD)
	@doxygen
	@echo Docs created at: ./build/doxy/html/index.html

hn-check:
	@~/home/ubuntu/llvm/install/bin/clang-tidy $(CHECK_SRC) \
		--checks="-*,readability-AfrHungarianVariables" \
		-- $(INC_DIRS)

hn-fix:
	@~/home/ubuntu/llvm/install/bin/clang-tidy $(CHECK_SRC) \
		--checks="-*,readability-AfrHungarianVariables" \
		-fix \
		-- $(INC_DIRS)

lib:
	@git clone https://github.com/ThrowTheSwitch/Unity.git $(PATH_LIB)unity
	@git --git-dir=$(PATH_LIB)unity/.git --work-tree=$(PATH_LIB)unity/ \
		checkout "v2.4.3"

list-obj:
	@echo OBJ_ALL $(OBJ_ALL)

list-src:
	@echo SRC_ALL $(SRC_ALL)
	@echo CHECK_SRC $(CHECK_SRC)

test: $(TGT)
	@./$(TGT) -v $(RUN_FLAGS) > $(RESULTS)
	@$(PASSED_TESTS)
	@$(IGNORED_TESTS)
	@$(FAILED_TESTS)
	@$(RESULT_SUMMARY)
	@$(FINAL_RESULT)

$(PATH_BUILD)default.profraw: $(TGT)
	@./$(TGT) -v $(RUN_FLAGS) > $(RESULTS)
	@mv default.profraw $(PATH_BUILD)default.profraw

$(PATH_BUILD)default.profdata: $(PATH_BUILD)default.profraw
	@llvm-profdata merge $(PATH_BUILD)default.profraw -o \
		$(PATH_BUILD)default.profdata

$(PATH_BUILD)%.o:: $(PATH_TEST)%.c $(HDR_ALL)
	$(dir_guard)
	$(COMPILE)

$(PATH_BUILD)%.o:: $(PATH_CBOR)%.c $(HDR_ALL)
	$(dir_guard)
	$(COMPILE_COV)

$(PATH_BUILD)%.o:: $(PATH_UNITY)%.c $(HDR_ALL)
	$(dir_guard)
	$(COMPILE)

$(PATH_BUILD)%.o:: $(PATH_UNITY_FIXT)%.c $(HDR_ALL)
	$(dir_guard)
	$(COMPILE)

$(TGT): $(OBJ_ALL)
	$(dir_guard)
	$(LINK)
