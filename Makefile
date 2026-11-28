TARGET   := sokobani
BUILD    ?= debug
CC       ?= cc
PREFIX   ?= /usr/local
WERROR   ?= 0

SRC_DIR   := src
TEST_DIR  := tests
BUILD_DIR := build/$(BUILD)

SRCS      := $(shell find $(SRC_DIR) -name '*.c')
OBJS      := $(SRCS:%.c=$(BUILD_DIR)/%.o)
LIB_OBJS  := $(filter-out $(BUILD_DIR)/$(SRC_DIR)/main.o,$(OBJS))
TEST_SRCS := $(wildcard $(TEST_DIR)/*.c)
TEST_OBJS := $(TEST_SRCS:%.c=$(BUILD_DIR)/%.o)
TEST_BINS := $(TEST_OBJS:.o=)
DEPS      := $(OBJS:.o=.d) $(TEST_OBJS:.o=.d)
BIN       := $(BUILD_DIR)/$(TARGET)

WARNINGS := -Wall -Wextra -Wpedantic -Wshadow -Wconversion -Wsign-conversion \
            -Wstrict-prototypes -Wmissing-prototypes -Wold-style-definition \
            -Wformat=2 -Wundef -Wcast-align -Wwrite-strings -Wpointer-arith \
            -Wnull-dereference -Wdouble-promotion

CPPFLAGS += -I$(SRC_DIR) -MMD -MP
CFLAGS   += -std=c11 $(WARNINGS)
LDFLAGS  +=
LDLIBS   +=

ifeq ($(BUILD),debug)
  CFLAGS += -O0 -g3
else ifeq ($(BUILD),release)
  CPPFLAGS += -DNDEBUG
  CFLAGS   += -O2
else ifeq ($(BUILD),sanitize)
  CFLAGS  += -O1 -g3 -fno-omit-frame-pointer -fsanitize=address,undefined -fno-sanitize-recover=all
  LDFLAGS += -fsanitize=address,undefined
else
  $(error Unknown BUILD '$(BUILD)', expected debug, release or sanitize)
endif

RAYLIB_VERSION := 5.5
RAYLIB_DIR     := build/deps/raylib-$(RAYLIB_VERSION)
RAYLIB_URL     := https://github.com/raysan5/raylib/archive/refs/tags/$(RAYLIB_VERSION).tar.gz
UNAME_S        := $(shell uname -s)

ifeq ($(shell pkg-config --exists raylib 2>/dev/null && echo yes),yes)
  RAYLIB_CFLAGS := $(shell pkg-config --cflags raylib)
  RAYLIB_LIBS   := $(shell pkg-config --libs raylib)
  RAYLIB_DEP    :=
  RAYLIB_HEADER :=
else
  RAYLIB_CFLAGS := -isystem $(RAYLIB_DIR)/src
  RAYLIB_DEP    := $(RAYLIB_DIR)/src/libraylib.a
  RAYLIB_HEADER := $(RAYLIB_DIR)/src/raylib.h
  ifeq ($(UNAME_S),Darwin)
    RAYLIB_LIBS := $(RAYLIB_DEP) -framework CoreVideo -framework IOKit \
                   -framework Cocoa -framework GLUT -framework OpenGL
  else
    RAYLIB_LIBS := $(RAYLIB_DEP) -lGL -lm -lpthread -ldl -lrt -lX11
  endif
endif

ifeq ($(WERROR),1)
  CFLAGS += -Werror
endif

FORMAT_FILES := $(shell find $(SRC_DIR) $(TEST_DIR) -name '*.[ch]')

.PHONY: all run test format format-check tidy install uninstall clean help raylib

all: $(BIN)

$(BIN): $(OBJS) $(RAYLIB_DEP)
	$(CC) $(LDFLAGS) $(OBJS) $(RAYLIB_LIBS) $(LDLIBS) -o $@

FONT_TTF    := assets/fonts/pixantiqua.ttf
FONT_HEADER := $(BUILD_DIR)/gen/font_data.h

$(BUILD_DIR)/$(SRC_DIR)/main.o: CPPFLAGS += $(RAYLIB_CFLAGS) -I$(BUILD_DIR)/gen
$(BUILD_DIR)/$(SRC_DIR)/main.o: $(FONT_HEADER) | $(RAYLIB_HEADER)

$(FONT_HEADER): $(FONT_TTF)
	@mkdir -p $(@D)
	{ echo 'static const unsigned char FONT_DATA[] = {'; \
	  od -An -v -tx1 $< | sed -e 's/\([0-9a-f][0-9a-f]\)/0x\1,/g'; \
	  echo '};'; } > $@

raylib: $(RAYLIB_DEP)

$(RAYLIB_DIR)/src/raylib.h:
	@mkdir -p build/deps
	curl -fsSL $(RAYLIB_URL) | tar xz -C build/deps
	@touch $@

$(RAYLIB_DIR)/src/libraylib.a: | $(RAYLIB_DIR)/src/raylib.h
	$(MAKE) -C $(RAYLIB_DIR)/src MAKEFLAGS= CC=$(CC) PLATFORM=PLATFORM_DESKTOP \
		RAYLIB_BUILD_MODE=RELEASE

$(BUILD_DIR)/%.o: %.c
	@mkdir -p $(@D)
	$(CC) $(CPPFLAGS) $(CFLAGS) -c $< -o $@

$(TEST_BINS): %: %.o $(LIB_OBJS)
	$(CC) $(LDFLAGS) $^ $(LDLIBS) -o $@

run: $(BIN)
	./$(BIN)

test: $(TEST_BINS)
	@set -e; for t in $(TEST_BINS); do echo "==> $$t"; ./$$t; done

format:
	clang-format -i $(FORMAT_FILES)

format-check:
	clang-format --dry-run --Werror $(FORMAT_FILES)

tidy: $(FONT_HEADER) | $(RAYLIB_HEADER)
	clang-tidy $(SRCS) $(TEST_SRCS) -- $(filter-out -MMD -MP,$(CPPFLAGS)) $(RAYLIB_CFLAGS) -I$(BUILD_DIR)/gen -std=c11

install: $(BIN)
	install -d $(DESTDIR)$(PREFIX)/bin
	install -m 755 $(BIN) $(DESTDIR)$(PREFIX)/bin/$(TARGET)

uninstall:
	rm -f $(DESTDIR)$(PREFIX)/bin/$(TARGET)

clean:
	rm -rf build

help:
	@echo "Targets: all run test raylib format format-check tidy install uninstall clean"
	@echo "Variables: BUILD=debug|release|sanitize CC=<compiler> WERROR=1 PREFIX=<path>"

-include $(DEPS)
