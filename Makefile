# https://stackoverflow.com/a/18258352
rwildcard=$(foreach d,$(wildcard $1*),$(call rwildcard,$d/,$2) $(filter $(subst *,%,$2),$d))

SRC_FILES = $(call rwildcard, src/, *.cpp)
OBJ_FILES = $(SRC_FILES:src/%.cpp=build/%.o)
DEP_FILES = $(OBJ_FILES:.o=.d)

# Use the emscripten compilers
CXX = em++
CC  = emcc

# also builds owop.wasm
TARGET    = out/owop.js

OPT_REL   = -O2 -s FILESYSTEM=0
LD_REL    = --closure 1 $(OPT_REL) # for post-compile emscripten stuff

LD_DBG   = -fsanitize=undefined -g4 -s ASSERTIONS=2 -s STACK_OVERFLOW_CHECK=2 -s DEMANGLE_SUPPORT=1 -s FILESYSTEM=1
OPT_DBG := -D DEBUG=1 $(LD_DBG)
LD_DBG  += --source-map-base "http://localhost:21002/vfs/1/9clH1bkfZxeEum8u/workspace/owop-client/"

EM_FEATURES = -s USE_LIBPNG=1 -s USE_SDL=0 -s MAX_WEBGL_VERSION=2
CPPFLAGS += $(EM_FEATURES)
LDFLAGS  += $(EM_FEATURES)

CPPFLAGS += -std=c++17 -fno-exceptions -W -Wall -Wextra -pedantic-errors -Wno-unused-parameter
CPPFLAGS += -MMD -MP

# GLM config
CPPFLAGS += -D GLM_FORCE_ARCH_UNKNOWN -D GLM_FORCE_CXX17 -D GLM_FORCE_PRECISION_MEDIUMP_FLOAT
CPPFLAGS += -I ./lib/glm/

# Libs to use
LDFLAGS  += -lGL

LDFLAGS  += -s ENVIRONMENT=web -s TOTAL_MEMORY=4MB -s TOTAL_STACK=32KB -s WEBGL2_BACKWARDS_COMPATIBILITY_EMULATION=1
LDFLAGS  += -s DISABLE_DEPRECATED_FIND_EVENT_TARGET_BEHAVIOR=1 -s ABORTING_MALLOC=0 #-s MALLOC=emmalloc
LDFLAGS  += -s EXPORT_NAME=AppOWOP -s MODULARIZE=1 -s STRICT=1

.PHONY: all rel dirs static clean

all: CPPFLAGS += $(OPT_DBG)
all: LDFLAGS += $(LD_DBG)
all: dirs static $(TARGET)

rel: CPPFLAGS += $(OPT_REL)
rel: LDFLAGS  += $(LD_REL)
rel: dirs static $(TARGET)

$(TARGET): $(OBJ_FILES)
	$(CXX) $(LDFLAGS) -o $@ $^ $(LDLIBS)

dirs:
	mkdir -p build out

static:
	cp -RT static/ out/

build/%.o: src/%.cpp
	$(CXX) $(CPPFLAGS) -c -o $@ $<

clean:
	- $(RM) -r build

-include $(DEP_FILES)
