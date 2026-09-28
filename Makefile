# Normally driven by build.py, which runs these targets inside the build image.
TARGET ?= host
CC_host := gcc
CC_aarch64 := aarch64-linux-gnu-gcc
CC_asan := gcc
CC_linux := gcc
CC_flatpak := gcc
CC_web := emcc
CC_windows := x86_64-w64-mingw32-gcc
CC_macos := clang
PKG_aarch64 := PKG_CONFIG_PATH=/usr/lib/aarch64-linux-gnu/pkgconfig PKG_CONFIG_LIBDIR=/usr/lib/aarch64-linux-gnu/pkgconfig
PKG_linux := PKG_CONFIG_PATH=/opt/sdl2/lib/pkgconfig
PKG_windows := PKG_CONFIG_PATH=/opt/sdl2/lib/pkgconfig
CC := $(CC_$(TARGET))
PKGCONF := $(PKG_$(TARGET)) pkg-config
OUT := build/$(TARGET)
BIN_host := $(OUT)/cyberworld
BIN_aarch64 := $(OUT)/cyberworld.aarch64
BIN_asan := $(OUT)/cyberworld
BIN_linux := $(OUT)/cyberworld
BIN_flatpak := $(OUT)/cyberworld
BIN_web := $(OUT)/cyberworld.js
BIN_windows := $(OUT)/cyberworld-endless.exe
BIN_macos := $(OUT)/cyberworld-endless
BIN := $(BIN_$(TARGET))

SRC_DIRS := $(sort $(dir $(wildcard src/*/*.c)))
SRCS := $(wildcard src/*/*.c)
OBJS := $(patsubst src/%.c,$(OUT)/obj/%.o,$(SRCS))

# The version the title shows: build.py passes its own (the tag, or the tag
# and the commits since); made alone, git's; else dev. A header written
# only when it changes, so only what shows it is built again.
VERSION ?= $(shell git describe --tags --match 'v*' 2>/dev/null | sed 's/^v//' | grep . || echo dev)
GEN := $(OUT)/gen
CFLAGS += -std=c11 -O2 -g -Wall -Wextra -Wno-unused-parameter -Wno-missing-field-initializers \
          -D_DEFAULT_SOURCE -MMD -MP $(addprefix -I,$(SRC_DIRS)) $(shell $(PKGCONF) --cflags sdl2)
# CI builds with WERROR=1: a warning in the game's own code fails the build
ifdef WERROR
CFLAGS += -Werror
endif
# the embedded GBA core, built into the image by docker/mgba.sh
MGBA_TARGET := $(if $(filter aarch64 web windows,$(TARGET)),$(TARGET),host)
ifeq ($(TARGET),web)
PKGCONF := true   # SDL2 comes from Emscripten's port (-sUSE_SDL=2)
endif
MGBA := /opt/mgba/$(MGBA_TARGET)
MGBA_LICENSE := /opt/mgba/LICENSE
# macOS (build.py macos, on a Mac): SDL2 and mGBA from macos/deps.sh, both
# universal and static, in MACOS_DEPS; one binary for Apple silicon and Intel
ifeq ($(TARGET),macos)
MACOS_DEPS ?= .build/macos-deps
PKGCONF := true
MGBA := $(MACOS_DEPS)
MGBA_LICENSE := $(MACOS_DEPS)/share/licenses/mGBA.txt
MAC_ARCH := -arch arm64 -arch x86_64 -mmacosx-version-min=11.0
CFLAGS += $(MAC_ARCH) $(shell $(MACOS_DEPS)/bin/sdl2-config --cflags)
endif
# the Flatpak (linux/flatpak/): the runtime's SDL2, the manifest's mGBA in /app
ifeq ($(TARGET),flatpak)
MGBA := /app
endif
CFLAGS += -I$(MGBA)/include
LDLIBS += $(shell $(PKGCONF) --libs sdl2) $(MGBA)/lib/libmgba.a -lpthread -lm
# the desktop builds: a window, the user's data folder (src/core/main.c)
ifneq ($(filter host asan linux flatpak windows macos,$(TARGET)),)
CFLAGS += -DCW_DESKTOP
endif
ifeq ($(TARGET),macos)
LDLIBS := $(MGBA)/lib/libmgba.a $(shell $(MACOS_DEPS)/bin/sdl2-config --static-libs) $(MAC_ARCH)
endif
# 64-bit Windows (docker/Dockerfile.windows, MinGW-w64): SDL2 and mGBA linked
# in, one .exe with no console window; its icon, manifest and version
# (windows/) as a resource. build.py passes FILE_VERSION, four numbers.
ifeq ($(TARGET),windows)
FILE_VERSION ?= 0,0,0,0
CFLAGS += -D_USE_MATH_DEFINES -D__USE_MINGW_ANSI_STDIO=1
LDLIBS := $(MGBA)/libmgba.a $(shell $(PKGCONF) --static --libs sdl2) -static -lshlwapi
OBJS += $(OUT)/obj/windows.res.o
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
$(OUT)/licenses/mGBA.txt: $(MGBA_LICENSE)
	@mkdir -p $(dir $@)
	cp $< $@

$(BIN): $(OBJS)
	$(CC) -o $@ $^ $(LDLIBS)

ifeq ($(TARGET),windows)
$(OUT)/obj/windows.res.o: windows/cyberworld-endless.rc windows/cyberworld-endless.manifest windows/icon.ico $(GEN)/version.h
	@mkdir -p $(dir $@)
	x86_64-w64-mingw32-windres -I$(GEN) -Iwindows -DCW_FILEVERSION=$(FILE_VERSION) -O coff -o $@ $<
endif

$(OUT)/obj/%.o: src/%.c | $(GEN)/version.h
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -I$(GEN) -c -o $@ $<

$(GEN)/version.h: FORCE
	@mkdir -p $(GEN)
	@printf '#define CW_VERSION "%s"\n' '$(VERSION)' > $@.new
	@if cmp -s $@.new $@; then rm -f $@.new; else mv $@.new $@; fi

FORCE:

clean:
	rm -rf build

-include $(OBJS:.o=.d)
.PHONY: all clean FORCE

# ROM-free unit tests (host only), with the address and undefined-behaviour
# sanitizers
TEST_SAN := -fsanitize=address,undefined -fno-sanitize-recover=all -fno-omit-frame-pointer
TEST_SRCS := tests/test_core.c src/core/rom.c src/core/pacing.c src/net/net_gen.c src/net/net_arena.c src/net/net_height.c src/net/net_shapes.c src/net/net_layouts.c src/net/net_route.c src/layer/navicust.c \
	src/layer/npc_lines.c src/layer/guardians.c src/core/rivals.c src/director/powers.c src/layer/text.c
build/host/test_core: $(TEST_SRCS) src/*/*.h
	@mkdir -p build/host
	$(CC_host) -std=c11 -O1 -g $(TEST_SAN) -Wall -Wextra -Wno-unused-parameter -D_DEFAULT_SOURCE $(if $(WERROR),-Werror) $(addprefix -I,$(SRC_DIRS)) -o $@ $(TEST_SRCS) -lm

test: build/host/test_core
	build/host/test_core
.PHONY: test
