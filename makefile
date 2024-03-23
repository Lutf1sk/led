OUT := led

SRC := \
	src/main.c \
	src/clipboard.c \
	src/clr.c \
	src/doc.c \
	src/hl.c \
	src/hl_c.c \
	src/hl_cpp.c \
	src/hl_rust.c \
	src/hl_cs.c \
	src/hl_onyx.c \
	src/hl_lpc.c \
	src/hl_js.c \
	src/hl_makefile.c \
	src/editor.c \
	src/command.c \
	src/file_browser.c \
	src/focus_terminal.c \
	src/focus_editor.c \
	src/focus_error.c \
	src/focus_browse_files.c \
	src/focus_browse_filesystem.c \
	src/focus_exit.c \
	src/focus_find_local.c \
	src/focus_command.c \
	src/focus.c \
	src/keybinds.c \
	src/draw.c

LT_PATH := lt
LT_ENV :=

# -----== COMPILER
CC := cc
CC_WARN := -Wall -Werror -Wno-strict-aliasing -Wno-error=unused-variable -Wno-unused-function -Wno-pedantic
CC_FLAGS := -I$(LT_PATH)/include/ -std=c11 -fmax-errors=3 $(CC_WARN) -mavx2 -masm=intel

ifdef WINDOWS
	CC = x86_64-w64-mingw32-gcc
endif

ifdef DEBUG
	CC_FLAGS += -fno-omit-frame-pointer -O0 -g
else
	CC_FLAGS += -O2
endif

# -----== LINKER
LNK := cc
LNK_LIBS := -lpthread -lm
LNK_FLAGS :=

ifdef WINDOWS
	LNK = x86_64-w64-mingw32-gcc
	LNK_LIBS += -lws2_32
	LNK_FLAGS += -static
else
	LNK_LIBS += -ldl
endif

ifdef DEBUG
	LNK_FLAGS += -lasan -lubsan -g -rdynamic
endif

LNK_FLAGS += $(LNK_LIBS)

# -----== TARGETS
ifdef DEBUG
	BIN_PATH := bin/debug
	LT_ENV += DEBUG=1
else
	BIN_PATH := bin/release
endif

OUT_PATH := $(BIN_PATH)/$(OUT)

LT_LIB := $(LT_PATH)/$(BIN_PATH)/lt.a

OBJS := $(patsubst %.c,$(BIN_PATH)/%.o,$(SRC))
DEPS := $(patsubst %.o,%.deps,$(OBJS))

all: lt $(OUT_PATH)

install: all
	cp $(OUT_PATH) /usr/local/bin/

run: all
	$(OUT_PATH) $(args)

clean:
	-rm -r $(BIN_PATH)

lt:
	$(LT_ENV) make -C $(LT_PATH)

$(OUT_PATH): $(OBJS) $(LT_LIB)
	$(LNK) $(OBJS) $(LT_LIB) $(LNK_FLAGS) -o $(OUT_PATH)

$(BIN_PATH)/%.o: %.c makefile
	@-mkdir -p $(BIN_PATH)/$(dir $<)
	@$(CC) $(CC_FLAGS) -MM -MT $@ -MF $(patsubst %.o,%.deps,$@) $<
	$(CC) $(CC_FLAGS) -c $< -o $@

-include $(DEPS)

.PHONY: all install run clean lt
