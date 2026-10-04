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
# (iOS: build.py ios, on a Mac with Xcode; the phone's SDK, or the Simulator's for CI's smoke test)
IOS_SDK ?= iphoneos
CC_ios := xcrun --sdk $(IOS_SDK) clang
CC_3ds := /opt/devkitpro/devkitARM/bin/arm-none-eabi-gcc
PKG_aarch64 := PKG_CONFIG_PATH=/usr/lib/aarch64-linux-gnu/pkgconfig PKG_CONFIG_LIBDIR=/usr/lib/aarch64-linux-gnu/pkgconfig
PKG_linux := PKG_CONFIG_PATH=/opt/sdl2/lib/pkgconfig
PKG_windows := PKG_CONFIG_PATH=/opt/sdl2/lib/pkgconfig
PKG_3ds := PKG_CONFIG_PATH=/opt/sdl2/lib/pkgconfig
CC := $(CC_$(TARGET))
PKGCONF := $(PKG_$(TARGET)) pkg-config
OUT := build/$(TARGET)$(if $(filter ios,$(TARGET)),-$(IOS_SDK))
BIN_host := $(OUT)/cyberworld
BIN_aarch64 := $(OUT)/cyberworld.aarch64
BIN_asan := $(OUT)/cyberworld
BIN_linux := $(OUT)/cyberworld
BIN_flatpak := $(OUT)/cyberworld
BIN_web := $(OUT)/cyberworld.js
BIN_windows := $(OUT)/cyberworld-endless.exe
BIN_macos := $(OUT)/cyberworld-endless
BIN_ios := $(OUT)/cyberworld-endless
BIN_3ds := $(OUT)/cyberworld-endless.elf
BIN := $(BIN_$(TARGET))

SRC_DIRS := $(sort $(dir $(wildcard src/*/*.c)))
SRCS := $(wildcard src/*/*.c)
OBJS := $(patsubst src/%.c,$(OUT)/obj/%.o,$(SRCS))

# The version the title shows: build.py passes its own (the tag, or the tag
# and the commits since); made alone, git's; else dev. A header written
# only when it changes, so only what shows it is built again.
VERSION ?= $(shell git describe --tags --match 'v*' 2>/dev/null | sed 's/^v//' | grep . || echo dev)
GEN := $(OUT)/gen
# The warnings (issue #19): -Wall -Wextra and stricter ones, clean on every
# target; the last three are GCC's own, which clang (Emscripten, macOS,
# the NDK) does not know. SDL2's and mGBA's headers are the system's.
WARN := -Wall -Wextra -Wno-unused-parameter -Wno-missing-field-initializers -Wshadow -Wmissing-prototypes \
        -Wstrict-prototypes -Wformat=2 -Wcast-qual -Wwrite-strings -Wundef -Wvla -Wnull-dereference -Wredundant-decls
WARN_GCC := -Wlogical-op -Wduplicated-cond -Wduplicated-branches
CFLAGS += -std=c11 -O2 -g $(WARN) $(if $(filter web macos ios,$(TARGET)),,$(WARN_GCC)) \
          -D_DEFAULT_SOURCE -MMD -MP $(addprefix -I,$(SRC_DIRS)) $(patsubst -I%,-isystem %,$(shell $(PKGCONF) --cflags sdl2))
# CI builds with WERROR=1: a warning in the game's own code fails the build
ifdef WERROR
CFLAGS += -Werror
endif
# the embedded GBA core, built into the image by docker/mgba.sh
MGBA_TARGET := $(if $(filter aarch64 web windows 3ds,$(TARGET)),$(TARGET),host)
ifeq ($(TARGET),web)
PKGCONF := true   # SDL2 comes from Emscripten's port (-sUSE_SDL=2)
endif
ifeq ($(TARGET),ios)
PKGCONF := true   # SDL2 comes from ios/deps.sh (sdl2-config), never the Mac's own
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
CFLAGS += $(MAC_ARCH) $(patsubst -I%,-isystem %,$(shell $(MACOS_DEPS)/bin/sdl2-config --cflags))
endif
# iOS (build.py ios, on a Mac with Xcode): SDL2 and mGBA from ios/deps.sh for
# IOS_SDK, static, arm64, iOS 14 on; ios.m is UIKit's (the ROM picker, the
# main SDL hands to UIKit)
ifeq ($(TARGET),ios)
IOS_DEPS ?= .build/ios-deps/$(IOS_SDK)
MGBA := $(IOS_DEPS)
MGBA_LICENSE := $(IOS_DEPS)/share/licenses/mGBA.txt
IOS_FLAGS := -arch arm64 -isysroot $(shell xcrun --sdk $(IOS_SDK) --show-sdk-path) \
             $(if $(filter iphonesimulator,$(IOS_SDK)),-mios-simulator-version-min=14.0,-miphoneos-version-min=14.0)
CFLAGS += $(IOS_FLAGS) -DCW_IOS $(patsubst -I%,-isystem %,$(shell $(IOS_DEPS)/bin/sdl2-config --cflags))
OBJS += $(OUT)/obj/core/ios.o
endif
# the Flatpak (linux/flatpak/): the runtime's SDL2, the manifest's mGBA in /app
ifeq ($(TARGET),flatpak)
MGBA := /app
endif
CFLAGS += -isystem $(MGBA)/include
LDLIBS += $(shell $(PKGCONF) --libs sdl2) $(MGBA)/lib/libmgba.a -lpthread -lm
# the desktop builds: a window, the user's data folder (src/core/main.c)
ifneq ($(filter host asan linux flatpak windows macos,$(TARGET)),)
CFLAGS += -DCW_DESKTOP
endif
ifeq ($(TARGET),macos)
LDLIBS := $(MGBA)/lib/libmgba.a $(shell $(MACOS_DEPS)/bin/sdl2-config --static-libs) $(MAC_ARCH)
endif
ifeq ($(TARGET),ios)
LDLIBS := $(MGBA)/lib/libmgba.a $(shell $(IOS_DEPS)/bin/sdl2-config --static-libs) -framework UniformTypeIdentifiers $(IOS_FLAGS)
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
          -sEXPORTED_RUNTIME_METHODS=callMain,ccall,FS,IDBFS,addRunDependency,removeRunDependency -sEXPORT_NAME=Module \
          -sEXPORTED_FUNCTIONS=_main,_cw_set_smooth
endif
# the Linux release: built on an older glibc (docker/Dockerfile.linux), SDL2
# carried in lib/ beside the binary
ifeq ($(TARGET),linux)
LDLIBS += -Wl,-rpath,'$$ORIGIN/lib'
endif
# the Nintendo 3DS (docker/Dockerfile.3ds, 3ds/, issue #9): devkitARM for
# the 3DS's ARM11, SDL2 and mGBA linked in, a .3dsx for the Homebrew
# Launcher with its title and icon, and the same game as a CIA the HOME
# Menu installs, with a banner (3ds/banner.png, tools/steam_art.py) that
# plays the trailer's opening hits
ifeq ($(TARGET),3ds)
ARCH_3DS := -march=armv6k -mtune=mpcore -mfloat-abi=hard -mtp=soft
# (-Wno-format: uint32_t is an unsigned long there, which %u prints alike)
CFLAGS := $(filter-out -g,$(CFLAGS)) $(ARCH_3DS) -mword-relocations -ffunction-sections -D__3DS__ -isystem /opt/devkitpro/libctru/include -Wno-format
LDLIBS := $(MGBA)/lib/libmgba.a -L/opt/sdl2/lib -lSDL2main -lSDL2 -L/opt/devkitpro/libctru/lib -lcitro2d -lcitro3d -lctru -lm \
          -specs=3dsx.specs $(ARCH_3DS) -Wl,--gc-sections
all: $(OUT)/cyberworld-endless.3dsx $(OUT)/cyberworld-endless.cia
$(OUT)/cyberworld-endless.smdh: 3ds/icon.png
	/opt/devkitpro/tools/bin/smdhtool --create "Cyberworld Endless" "A Mega Man Battle Network 6 roguelike" "saschb2b" $< $@
$(OUT)/cyberworld-endless.3dsx: $(BIN) $(OUT)/cyberworld-endless.smdh
	/opt/devkitpro/tools/bin/3dsxtool $(BIN) $@ --smdh=$(OUT)/cyberworld-endless.smdh
$(OUT)/banner.wav: tools/trailer_music.py
	python3 tools/trailer_music.py $@ --seconds 2.9 --rate 32728
$(OUT)/banner.bnr: 3ds/banner.png $(OUT)/banner.wav
	bannertool makebanner -i 3ds/banner.png -a $(OUT)/banner.wav -o $@
$(OUT)/cyberworld-endless.cia: $(BIN) 3ds/cia.rsf $(OUT)/cyberworld-endless.smdh $(OUT)/banner.bnr
	makerom -f cia -o $@ -rsf 3ds/cia.rsf -target t -exefslogo -elf $(BIN) -icon $(OUT)/cyberworld-endless.smdh -banner $(OUT)/banner.bnr
endif
ifeq ($(TARGET),asan)
CFLAGS += -O1 -fsanitize=address,undefined -fno-omit-frame-pointer
LDLIBS += -fsanitize=address,undefined
endif

# one make's own flags on top (build.py lint: each function in a section
# of its own, and the linker's list of those it drops)
CFLAGS += $(EXTRA_CFLAGS)
LDLIBS += $(EXTRA_LDFLAGS)

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

# (UIKit's, iOS alone: Objective-C with ARC)
$(OUT)/obj/%.o: src/%.m | $(GEN)/version.h
	@mkdir -p $(dir $@)
	$(CC) $(filter-out -std=c11,$(CFLAGS)) -fobjc-arc -I$(GEN) -c -o $@ $<

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
TEST_SRCS := tests/test_core.c src/core/rom.c src/core/pacing.c src/net/net_gen.c src/net/net_pieces.c src/net/net_arena.c src/net/net_height.c src/net/net_shapes.c src/net/net_layouts.c src/net/net_route.c src/net/net_way.c src/layer/navicust.c \
	src/layer/npc_lines.c src/layer/guardians.c src/core/rivals.c src/director/powers.c src/layer/text.c src/core/touch_layout.c src/audio/xsong.c src/layer/xnavi.c \
	src/core/data.c
build/host/test_core: $(TEST_SRCS) src/*/*.h
	@mkdir -p build/host
	$(CC_host) -std=c11 -O1 -g $(TEST_SAN) $(WARN) $(WARN_GCC) -D_DEFAULT_SOURCE $(if $(WERROR),-Werror) $(addprefix -I,$(SRC_DIRS)) -o $@ $(TEST_SRCS) -lm

# the hooks on mGBA itself (tests/test_emu.c: a ROM of the test's own bytes)
TEST_EMU_SRCS := tests/test_emu.c src/emu/hook.c
build/host/test_emu: $(TEST_EMU_SRCS) src/emu/hook.h
	@mkdir -p build/host
	$(CC_host) -std=c11 -O1 -g $(TEST_SAN) $(WARN) $(WARN_GCC) -D_DEFAULT_SOURCE $(if $(WERROR),-Werror) -Isrc/emu -isystem $(MGBA)/include \
		-o $@ $(TEST_EMU_SRCS) $(MGBA)/lib/libmgba.a -lpthread -lm

test: build/host/test_core build/host/test_emu
	build/host/test_core
	build/host/test_emu
.PHONY: test

# the tests' link as build.py lint reads it: which of the game's functions
# they reach (each source compiled on its own, so the linker names it)
LINT_TEST_OBJS := $(patsubst %.c,build/lint/test/%.o,$(TEST_SRCS))
LINT_TEST_EMU_OBJS := $(patsubst %.c,build/lint/test/%.o,$(TEST_EMU_SRCS))
build/lint/test/%.o: %.c
	@mkdir -p $(dir $@)
	$(CC_host) -std=c11 -O1 -D_DEFAULT_SOURCE $(addprefix -I,$(SRC_DIRS)) -isystem $(MGBA)/include -ffunction-sections -c -o $@ $<
build/lint/test_core: $(LINT_TEST_OBJS)
	$(CC_host) -o $@ $^ -Wl,--gc-sections -Wl,--print-gc-sections -lm
build/lint/test_emu: $(LINT_TEST_EMU_OBJS)
	$(CC_host) -o $@ $^ -Wl,--gc-sections -Wl,--print-gc-sections $(MGBA)/lib/libmgba.a -lpthread -lm
