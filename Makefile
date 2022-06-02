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
TARGET = $(OUT_DIR)/owop.js

ifndef VERSION
TREE_STATE = $(shell git rev-parse --short HEAD)
ifeq ($(shell git diff --quiet HEAD; echo $$?), 1)
	TREE_STATE := $(TREE_STATE)-$(shell ./git-hash.sh | cut -c -7)
endif
else
TREE_STATE = $(VERSION)
endif

OPT_REL += -O3 -ffast-math
# for post-compile emscripten stuff
LD_REL  += -s GL_TRACK_ERRORS=0 --closure 1 $(OPT_REL)

OPT_DBG += -g
LD_DBG  += $(OPT_DBG) -s ASSERTIONS=2 -s STACK_OVERFLOW_CHECK=2 -s DEMANGLE_SUPPORT=1 -gsource-map --source-map-base "/"
OPT_DBG += -D DEBUG=1
ifdef DISABLE_AUTO_REFRESH
OPT_DBG += -D DISABLE_AUTO_REFRESH=1
endif

EM_CONF_CC_LD += -s STRICT=1 -s USE_LIBPNG=1 -s USE_SDL=0
EM_CONF_LD += -s USE_SDL_MIXER=0 -s USE_GLFW=0 -s USE_SDL_IMAGE=0 -s USE_SDL_TTF=0 -s USE_SDL_NET=0 -s FILESYSTEM=0
EM_CONF_LD += -s MAX_WEBGL_VERSION=2

EM_CONF_LD += -s GL_SUPPORT_AUTOMATIC_ENABLE_EXTENSIONS=0 -s GL_SUPPORT_SIMPLE_ENABLE_EXTENSIONS=0
EM_CONF_LD += -s GL_EXTENSIONS_IN_PREFIXED_FORMAT=0 -s GL_EMULATE_GLES_VERSION_STRING_FORMAT=0

EM_CONF_LD += -s INITIAL_MEMORY=8MB -s TOTAL_STACK=32KB -s ALLOW_MEMORY_GROWTH=0 -s ABORTING_MALLOC=0 #-s MALLOC=emmalloc
EM_CONF_LD += -s FETCH_SUPPORT_INDEXEDDB=0 -s WEBGL2_BACKWARDS_COMPATIBILITY_EMULATION=1
EM_CONF_LD += -s HTML5_SUPPORT_DEFERRING_USER_SENSITIVE_REQUESTS=0
EM_CONF_LD += -s INCOMING_MODULE_JS_API=[] -s EXPORTED_RUNTIME_METHODS=['callMain']

EM_CONF_LD += -s ENVIRONMENT=web -s EXPORT_NAME=AppOWOP -s MODULARIZE=1 -s INVOKE_RUN=0

CPPFLAGS += $(EM_CONF_CC_LD)
LDFLAGS  += $(EM_CONF_CC_LD) $(EM_CONF_LD)


CPPFLAGS += -std=c++17 -fno-exceptions -Wall -Wshadow -Weffc++ -Wextra -pedantic-errors -Wno-unused-parameter -MMD -MP
LDFLAGS  += -fno-exceptions

# GLM config
CPPFLAGS += -D GLM_FORCE_ARCH_UNKNOWN -D GLM_FORCE_PRECISION_MEDIUMP_FLOAT
CPPFLAGS += -I ./lib/glm/

CPPFLAGS += -I ./src/

# Libs to use
LDFLAGS  += -lGL


.PHONY: all dbg rel static clean

all: dbg

dbg: TREE_STATE := $(TREE_STATE)-dbg
dbg: CPPFLAGS += $(OPT_DBG) -D OWOP_VERSION='"$(TREE_STATE)"'
dbg: LDFLAGS += $(LD_DBG)
dbg: static $(TARGET)

rel: TREE_STATE := $(TREE_STATE)-rel
rel: CPPFLAGS += $(OPT_REL) -D OWOP_VERSION='"$(TREE_STATE)"'
rel: LDFLAGS  += $(LD_REL)
rel: static $(TARGET)

$(TARGET): $(OBJ_FILES)
	$(CXX) $(LDFLAGS) -o $@ $^ $(LDLIBS)

static: $(OUT_DIR)
	cp -RT $(STATIC_DIR)/ $(OUT_DIR)/

.SECONDEXPANSION:
$(OBJ_DIR)/%.o: $(SRC_DIR)/%.cpp | $$(@D)
	$(CXX) $(CPPFLAGS) -c -o $@ $<

$(OUT_DIR) $(patsubst %/,%,$(sort $(dir $(OBJ_FILES)))):
	@mkdir -p $@

clean:
	- $(RM) -r $(OBJ_DIR) $(OUT_DIR)/*

-include $(DEP_FILES)
