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

OPT_REL   = -O2
LD_REL    = --closure 1 -O2 # for post-compile emscripten stuff

OPT_DBG  = -g4 -s ASSERTIONS=2 -s STACK_OVERFLOW_CHECK=2 -s DEMANGLE_SUPPORT=1
LD_DBG   = -g4 -s ASSERTIONS=2 -s STACK_OVERFLOW_CHECK=2 -s DEMANGLE_SUPPORT=1

EM_FEATURES = #-s USE_LIBPNG=1 -s USE_WEBGL2=1
CPPFLAGS += $(EM_FEATURES)
LDFLAGS  += $(EM_FEATURES)

CPPFLAGS += -std=c++17 -fno-exceptions
CPPFLAGS += -MMD -MP
LDFLAGS  += -s ENVIRONMENT=web #-s WEBGL2_BACKWARDS_COMPATIBILITY_EMULATION=1

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
