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

ifeq ($(WERROR),1)
  CFLAGS += -Werror
endif

FORMAT_FILES := $(shell find $(SRC_DIR) $(TEST_DIR) -name '*.[ch]')

.PHONY: all run test format format-check tidy install uninstall clean help

all: $(BIN)

$(BIN): $(OBJS)
	$(CC) $(LDFLAGS) $^ $(LDLIBS) -o $@

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

tidy:
	clang-tidy $(SRCS) $(TEST_SRCS) -- $(filter-out -MMD -MP,$(CPPFLAGS)) -std=c11

install: $(BIN)
	install -d $(DESTDIR)$(PREFIX)/bin
	install -m 755 $(BIN) $(DESTDIR)$(PREFIX)/bin/$(TARGET)

uninstall:
	rm -f $(DESTDIR)$(PREFIX)/bin/$(TARGET)

clean:
	rm -rf build

help:
	@echo "Targets: all run test format format-check tidy install uninstall clean"
	@echo "Variables: BUILD=debug|release|sanitize CC=<compiler> WERROR=1 PREFIX=<path>"

-include $(DEPS)
