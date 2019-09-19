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
LD_REL    = -s

OPT_DBG  = -g3
LD_DBG   =

CPPFLAGS += -std=c++17
CPPFLAGS += -MMD -MP

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
