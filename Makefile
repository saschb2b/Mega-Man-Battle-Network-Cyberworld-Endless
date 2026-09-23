# Normally driven by build.py, which runs these targets inside the build image.
TARGET ?= host
CC_host := gcc
CC_aarch64 := aarch64-linux-gnu-gcc
CC_asan := gcc
PKG_aarch64 := PKG_CONFIG_PATH=/usr/lib/aarch64-linux-gnu/pkgconfig PKG_CONFIG_LIBDIR=/usr/lib/aarch64-linux-gnu/pkgconfig
CC := $(CC_$(TARGET))
PKGCONF := $(PKG_$(TARGET)) pkg-config
OUT := build/$(TARGET)
BIN_host := $(OUT)/cyberworld
BIN_aarch64 := $(OUT)/cyberworld.aarch64
BIN_asan := $(OUT)/cyberworld
BIN := $(BIN_$(TARGET))

SRC_DIRS := $(sort $(dir $(wildcard src/*/*.c)))
SRCS := $(wildcard src/*/*.c)
OBJS := $(patsubst src/%.c,$(OUT)/obj/%.o,$(SRCS))
CFLAGS += -std=c11 -O2 -g -Wall -Wextra -Wno-unused-parameter -Wno-missing-field-initializers \
          -D_DEFAULT_SOURCE -MMD -MP $(addprefix -I,$(SRC_DIRS)) $(shell $(PKGCONF) --cflags sdl2)
LDLIBS += $(shell $(PKGCONF) --libs sdl2) -lm
ifeq ($(TARGET),asan)
CFLAGS += -O1 -fsanitize=address,undefined -fno-omit-frame-pointer
LDLIBS += -fsanitize=address,undefined
endif

all: $(BIN)

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
TEST_SRCS := tests/test_core.c src/core/rom.c src/net/net_gen.c
build/host/test_core: $(TEST_SRCS) src/*/*.h
	@mkdir -p build/host
	$(CC_host) -std=c11 -O1 -g -Wall -Wextra -Wno-unused-parameter -D_DEFAULT_SOURCE $(addprefix -I,$(SRC_DIRS)) -o $@ $(TEST_SRCS)

test: build/host/test_core
	build/host/test_core
.PHONY: test
