CC := C:/mingw64/bin/gcc.exe
RAYLIB_DIR := C:/raylib/6_0

# debug | release  -- override on the command line: mingw32-make CONFIG=release
CONFIG ?= debug

# -MMD -MP: emit .d files so objects rebuild when an included header changes
CFLAGS_COMMON := -Wall -std=c99 -MMD -MP -I$(RAYLIB_DIR)/include
LDLIBS := -L$(RAYLIB_DIR)/lib -lraylib -lopengl32 -lgdi32 -lwinmm

ifeq ($(CONFIG),debug)
  # DEBUG_MODE: enables debug logging, e.g. the board dump after every turn
  CFLAGS := $(CFLAGS_COMMON) -g -O0 -DDEBUG_MODE
else ifeq ($(CONFIG),release)
  CFLAGS := $(CFLAGS_COMMON) -O2 -DNDEBUG
  # -s: strip symbols
  LDFLAGS := -s
else
  $(error CONFIG must be 'debug' or 'release', got '$(CONFIG)')
endif

BUILDDIR := build/$(CONFIG)
SRC		 := $(wildcard src/*.c)
OBJ 	 := $(SRC:src/%.c=$(BUILDDIR)/%.o)
TARGET 	 := $(BUILDDIR)/clay_2048.exe

# cmd.exe needs backslashes for mkdir
BUILDDIR_WIN := $(subst /,\,$(BUILDDIR))

.PHONY: all debug release clean

all: $(TARGET)

debug:
	$(MAKE) CONFIG=debug

release:
	$(MAKE) CONFIG=release

$(TARGET): $(OBJ)
	$(CC) $(OBJ) -o $@ $(LDFLAGS) $(LDLIBS)

-include $(OBJ:.o=.d)

$(BUILDDIR)/%.o: src/%.c | $(BUILDDIR)
	$(CC) $(CFLAGS) -c $< -o $@

$(BUILDDIR):
	if not exist $(BUILDDIR_WIN) mkdir $(BUILDDIR_WIN)

clean:
	if exist build rmdir /s /q build
