# Normally driven by build.py, which runs these targets inside the build image.
TARGET ?= host
CC_host := gcc
CC_aarch64 := aarch64-linux-gnu-gcc
CC_asan := gcc
CC_linux := gcc
CC_web := emcc
PKG_aarch64 := PKG_CONFIG_PATH=/usr/lib/aarch64-linux-gnu/pkgconfig PKG_CONFIG_LIBDIR=/usr/lib/aarch64-linux-gnu/pkgconfig
PKG_linux := PKG_CONFIG_PATH=/opt/sdl2/lib/pkgconfig
CC := $(CC_$(TARGET))
PKGCONF := $(PKG_$(TARGET)) pkg-config
OUT := build/$(TARGET)
BIN_host := $(OUT)/cyberworld
BIN_aarch64 := $(OUT)/cyberworld.aarch64
BIN_asan := $(OUT)/cyberworld
BIN_linux := $(OUT)/cyberworld
BIN_web := $(OUT)/cyberworld.js
BIN := $(BIN_$(TARGET))

SRC_DIRS := $(sort $(dir $(wildcard src/*/*.c)))
SRCS := $(wildcard src/*/*.c)
OBJS := $(patsubst src/%.c,$(OUT)/obj/%.o,$(SRCS))
CFLAGS += -std=c11 -O2 -g -Wall -Wextra -Wno-unused-parameter -Wno-missing-field-initializers \
          -D_DEFAULT_SOURCE -MMD -MP $(addprefix -I,$(SRC_DIRS)) $(shell $(PKGCONF) --cflags sdl2)
# the embedded GBA core, built into the image by docker/mgba.sh
MGBA_TARGET := $(if $(filter aarch64 web,$(TARGET)),$(TARGET),host)
ifeq ($(TARGET),web)
PKGCONF := true   # SDL2 comes from Emscripten's port (-sUSE_SDL=2)
endif
MGBA := /opt/mgba/$(MGBA_TARGET)
CFLAGS += -I$(MGBA)/include
LDLIBS += $(shell $(PKGCONF) --libs sdl2) $(MGBA)/lib/libmgba.a -lpthread -lm
# the desktop builds: a window, the user's data folder (src/core/main.c)
ifneq ($(filter host asan linux,$(TARGET)),)
CFLAGS += -DCW_DESKTOP
endif
# the browser build (docker/Dockerfile.web, web/): one thread, the page
# drives the frames, files kept in IndexedDB
ifeq ($(TARGET),web)
CFLAGS := $(filter-out -g,$(CFLAGS)) -sUSE_SDL=2 -DDISABLE_THREADING
LDLIBS += -O2 -sUSE_SDL=2 -sALLOW_MEMORY_GROWTH=1 -sINITIAL_MEMORY=64MB -sSTACK_SIZE=1MB -lidbfs.js \
          -sINVOKE_RUN=0 -sEXIT_RUNTIME=0 -sFORCE_FILESYSTEM=1 -sENVIRONMENT=web \
          -sEXPORTED_RUNTIME_METHODS=callMain,FS,IDBFS,addRunDependency,removeRunDependency -sEXPORT_NAME=Module
endif
# the Linux release: built on an older glibc (docker/Dockerfile.linux), SDL2
# carried in lib/ beside the binary
ifeq ($(TARGET),linux)
LDLIBS += -Wl,-rpath,'$$ORIGIN/lib'
endif
ifeq ($(TARGET),asan)
CFLAGS += -O1 -fsanitize=address,undefined -fno-omit-frame-pointer
LDLIBS += -fsanitize=address,undefined
endif

all: $(BIN) $(OUT)/licenses/mGBA.txt

# mGBA is MPL-2.0; its license ships with the port (source: github.com/mgba-emu/mgba, tag 0.10.5)
$(OUT)/licenses/mGBA.txt: /opt/mgba/LICENSE
	@mkdir -p $(dir $@)
	cp $< $@

$(BIN): $(OBJS)
	$(CC) -o $@ $^ $(LDLIBS)

$(OUT)/obj/%.o: src/%.c
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -c -o $@ $<

clean:
	rm -rf build

-include $(OBJS:.o=.d)
.PHONY: all clean

# ROM-free unit tests (host only)
TEST_SRCS := tests/test_core.c src/core/rom.c src/core/pacing.c src/net/net_gen.c src/net/net_arena.c src/net/net_height.c src/net/net_shapes.c src/net/net_layouts.c
build/host/test_core: $(TEST_SRCS) src/*/*.h
	@mkdir -p build/host
	$(CC_host) -std=c11 -O1 -g -Wall -Wextra -Wno-unused-parameter -D_DEFAULT_SOURCE $(addprefix -I,$(SRC_DIRS)) -o $@ $(TEST_SRCS)

test: build/host/test_core
	build/host/test_core
.PHONY: test
