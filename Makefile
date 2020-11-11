# https://stackoverflow.com/a/18258352
rwildcard=$(foreach d,$(wildcard $1*),$(call rwildcard,$d/,$2) $(filter $(subst *,%,$2),$d))

SRC_DIR = src
STATIC_DIR = static

OUT_DIR = out
OBJ_DIR = build

SRC_FILES = $(call rwildcard, $(SRC_DIR)/, *.cpp)
OBJ_FILES = $(SRC_FILES:$(SRC_DIR)/%.cpp=$(OBJ_DIR)/%.o)
DEP_FILES = $(OBJ_FILES:.o=.d)

# Use the emscripten compilers
CXX = em++
CC  = emcc

# also builds owop.wasm
TARGET = owop.js

TREE_STATE = $(shell git rev-parse --short HEAD)
ifeq ($(shell git diff --quiet HEAD; echo $$?), 1)
	TREE_STATE := $(TREE_STATE)-$(shell ./git-hash.sh | cut -c -7)
endif

OPT_REL   = -O2 -s FILESYSTEM=0
# for post-compile emscripten stuff
LD_REL    = --closure 1 $(OPT_REL)

LD_DBG   = -fsanitize=undefined -g4 -s ASSERTIONS=2 -s STACK_OVERFLOW_CHECK=2 -s DEMANGLE_SUPPORT=1 -s FILESYSTEM=1
OPT_DBG := -D DEBUG=1 $(LD_DBG)
#LD_DBG  += --source-map-base "http://localhost:21002/vfs/1/9clH1bkfZxeEum8u/workspace/owop-client/"

EM_FEATURES = -s USE_LIBPNG=1 -s USE_SDL=0 -s MAX_WEBGL_VERSION=2
CPPFLAGS += $(EM_FEATURES)
LDFLAGS  += $(EM_FEATURES)

CPPFLAGS += -std=c++17 -fno-exceptions -W -Wall -Wextra -pedantic-errors -Wno-unused-parameter
CPPFLAGS += -MMD -MP

# GLM config
CPPFLAGS += -D GLM_FORCE_ARCH_UNKNOWN -D GLM_FORCE_CXX17 -D GLM_FORCE_PRECISION_MEDIUMP_FLOAT
CPPFLAGS += -I ./lib/glm/

CPPFLAGS += -I ./src/

# Libs to use
LDFLAGS  += -lGL

LDFLAGS  += -s ENVIRONMENT=web -s INITIAL_MEMORY=8MB -s TOTAL_STACK=32KB -s WEBGL2_BACKWARDS_COMPATIBILITY_EMULATION=1
LDFLAGS  += -s ALLOW_MEMORY_GROWTH=0 -s ABORTING_MALLOC=0 #-s MALLOC=emmalloc
LDFLAGS  += -s ABORT_ON_WASM_EXCEPTIONS=1 -s HTML5_SUPPORT_DEFERRING_USER_SENSITIVE_REQUESTS=0
LDFLAGS  += -s EXPORT_NAME=AppOWOP -s MODULARIZE=1 -s STRICT=1

.PHONY: all dbg rel static clean

all: dbg

dbg: TREE_STATE := $(TREE_STATE)-dbg
dbg: CPPFLAGS += $(OPT_DBG) -D VERSION='"$(TREE_STATE)"'
dbg: LDFLAGS += $(LD_DBG)
dbg: static $(TARGET)

rel: TREE_STATE := $(TREE_STATE)-rel
rel: CPPFLAGS += $(OPT_REL) -D VERSION='"$(TREE_STATE)"'
rel: LDFLAGS  += $(LD_REL)
rel: static $(TARGET)

$(TARGET): $(OBJ_FILES)
	$(CXX) $(LDFLAGS) -o $(OUT_DIR)/$@ $^ $(LDLIBS)

static: $(OUT_DIR)
	cp -RT $(STATIC_DIR)/ $(OUT_DIR)/

.SECONDEXPANSION:
$(OBJ_DIR)/%.o: $(SRC_DIR)/%.cpp | $$(@D)
	$(CXX) $(CPPFLAGS) -c -o $@ $<

$(OUT_DIR) $(patsubst %/,%,$(sort $(dir $(OBJ_FILES)))):
	@mkdir -p $@

clean:
	- $(RM) -r $(OBJ_DIR) $(OUT_DIR)

-include $(DEP_FILES)
