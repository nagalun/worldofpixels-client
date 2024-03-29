# https://stackoverflow.com/a/18258352
rwildcard=$(foreach d,$(wildcard $1*),$(call rwildcard,$d/,$2) $(filter $(subst *,%,$2),$d))

SRC_DIR = src
STATIC_DIR = static

OBJ_DIR = build
OUT_DIR = out

STATIC_PRE_FILES = $(call rwildcard, $(STATIC_DIR)/, *.html) $(call rwildcard, $(STATIC_DIR)/, *.js)
STATIC_OUT_FILES = $(STATIC_PRE_FILES:$(STATIC_DIR)/%=$(OUT_DIR)/%)
STATIC_DEP_FILES = $(STATIC_PRE_FILES:%=$(OBJ_DIR)/%.d)

SRC_FILES = $(call rwildcard, $(SRC_DIR)/, *.cpp)
OBJ_FILES = $(SRC_FILES:$(SRC_DIR)/%.cpp=$(OBJ_DIR)/%.o)
DEP_FILES = $(OBJ_FILES:.o=.d)
DWO_FILES = $(OBJ_FILES:.o=.dwo)

# Use the emscripten toolchain
CXX = em++
CC  = emcc
DWP = emdwp

# Preprocessor for static files
PP = cpp
PPFLAGS = -traditional-cpp -nostdinc -undef -I "$(STATIC_DIR)/preprocessor/" -imacros macros.h -P -E -MMD -MP

ifndef VERSION
TREE_STATE = $(shell git rev-parse --short HEAD)
ifeq ($(shell git diff --quiet HEAD; echo $$?), 1)
	TREE_STATE := $(TREE_STATE)-$(shell ./git-hash.sh | cut -c -7)
endif
else
TREE_STATE = $(VERSION)
endif

# also builds owop.wasm
TARGET = $(OUT_DIR)/js/owop-$(TREE_STATE).js

OPT_REL += -O3 -ffast-math -flto
# for post-compile emscripten stuff
LD_REL  += -s GL_TRACK_ERRORS=0 --closure 1 $(OPT_REL)

OPT_DBG += -O0 -g -gdwarf-5 -gsplit-dwarf -gpubnames
OPT_DBG += -ffile-prefix-map=$(CURDIR)=.
#-gseparate-dwarf=$(TARGET:.js=.dbg.wasm)
LD_DBG  += $(OPT_DBG)
LD_DBG  += -s ERROR_ON_WASM_CHANGES_AFTER_LINK=1 -s ASSERTIONS=1 -s WASM_BIGINT=1

OPT_LDBG += -O2 -g2
OPT_LDBG += -ffile-prefix-map=$(CURDIR)=.
LD_LDBG  += $(OPT_LDBG)
LD_LDBG  += -s ASSERTIONS=1

ifdef DISABLE_AUTO_REFRESH
OPT_DBG += -D DISABLE_AUTO_REFRESH=1
DEFS += -D DISABLE_AUTO_REFRESH=1
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


CPPFLAGS += -std=c++20 -fno-exceptions -fno-rtti -MMD -MP
CPPFLAGS += -Wall -Wshadow -Weffc++ -Wextra -pedantic-errors -Wno-unused-parameter
LDFLAGS  += -fno-exceptions -fno-rtti

# GLM config
CPPFLAGS += -D GLM_FORCE_ARCH_UNKNOWN -D GLM_FORCE_PRECISION_MEDIUMP_FLOAT
CPPFLAGS += -I ./lib/

CPPFLAGS += -iquote ./src/

# Libs to use
LDFLAGS  += -lGL


.PHONY: all dbg udbg ldbg rel static clean .FORCE

.FORCE:

all: ldbg

udbg: DEFS += -D OWOP_VERSION='$(TREE_STATE)' -D DEBUG=1
udbg: CPPFLAGS += $(OPT_DBG) $(DEFS) -fsanitize=address,undefined
udbg: LDFLAGS += $(LD_DBG) -fsanitize=address,undefined -s INITIAL_MEMORY=128MB -sERROR_ON_UNDEFINED_SYMBOLS=0
udbg: PPFLAGS += $(DEFS)
udbg: static $(TARGET)

dbg: DEFS += -D OWOP_VERSION='$(TREE_STATE)' -D DEBUG=1
dbg: CPPFLAGS += $(OPT_DBG) $(DEFS)
dbg: LDFLAGS += $(LD_DBG)
dbg: PPFLAGS += $(DEFS)
dbg: static $(TARGET)

# light debug
ldbg: DEFS += -D OWOP_VERSION='$(TREE_STATE)' -D DEBUG=1
ldbg: CPPFLAGS += $(OPT_LDBG) $(DEFS)
ldbg: LDFLAGS += $(LD_LDBG)
ldbg: PPFLAGS += $(DEFS)
ldbg: static $(TARGET)

rel: DEFS += -D OWOP_VERSION='$(TREE_STATE)'
rel: CPPFLAGS += $(OPT_REL) $(DEFS)
rel: LDFLAGS  += $(LD_REL)
rel: PPFLAGS += $(DEFS)
rel: static $(TARGET)

$(TARGET): $(OBJ_FILES) build/jute.o
	$(CXX) $(LDFLAGS) -o $@ $^ $(LDLIBS)
	$(DWP) -e $(@:.js=.wasm) -o $(@:.js=.wasm.dwp)

static: $(OUT_DIR) $(STATIC_DIR)/preprocessor/static_files.txt $(STATIC_OUT_FILES)
	cp -RTu $(STATIC_DIR)/ $(OUT_DIR)/
	- $(RM) -rf $(filter-out $(wildcard $(TARGET:.js=)*),$(wildcard $(OUT_DIR)/js/owop-*)) ./$(OUT_DIR)/preprocessor

$(STATIC_DIR)/preprocessor/static_files.txt: $(filter-out $(call rwildcard, $(STATIC_DIR)/preprocessor, *),$(call rwildcard, $(STATIC_DIR)/, *))
	find $(STATIC_DIR)/ -type f -not -path '*/preprocessor/*' -printf '/%P\n' > $(STATIC_DIR)/preprocessor/static_files.txt
	find $(STATIC_DIR)/theme/ -mindepth 1 -maxdepth 1 -type d -printf '"%P",' | sed 's/^/\[/;s/,$$/\]\n/' > $(STATIC_DIR)/theme/builtin.json

$(OUT_DIR)/%: $(STATIC_DIR)/%
	@mkdir -p $(dir $(OBJ_DIR)/$<) $(dir $(<:$(STATIC_DIR)/%=$(OUT_DIR)/%))
	$(PP) $(PPFLAGS) -MT $@ -MF $(OBJ_DIR)/$<.d $< $@
	@sed -i '1 s,:,: $$(TARGET) ,' $(OBJ_DIR)/$<.d

.SECONDEXPANSION:
$(OBJ_DIR)/%.o: $(SRC_DIR)/%.cpp | $$(@D)
	$(CXX) $(CPPFLAGS) -c -o $@ $<

# force build of main to update version/date, and fix cache bug when c++ code has no changes
$(OBJ_DIR)/main.o: .FORCE

$(OUT_DIR) $(patsubst %/,%,$(sort $(dir $(OBJ_FILES)))):
	@mkdir -p $@

build/jute.o: lib/json/jute.cpp
	$(CXX) $(CPPFLAGS) -c -o $@ $<

clean:
	- $(RM) -r $(OBJ_DIR) ./$(OUT_DIR)/* $(STATIC_DIR)/preprocessor/static_files.txt $(STATIC_DIR)/theme/builtin.json

-include $(DEP_FILES) $(STATIC_DEP_FILES)
