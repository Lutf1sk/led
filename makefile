OUT = led

OBJS = \
	src/main.o \
	src/clipboard.o \
	src/clr.o \
	src/doc.o \
	src/hl.o \
	src/hl_c.o \
	src/hl_cpp.o \
	src/hl_rust.o \
	src/hl_cs.o \
	src/hl_onyx.o \
	src/hl_lpc.o \
	src/hl_js.o \
	src/editor.o \
	src/command.o \
	src/file_browser.o \
	src/focus_terminal.o \
	src/focus_editor.o \
	src/focus_error.o \
	src/focus_browse_files.o \
	src/focus_browse_filesystem.o \
	src/focus_exit.o \
	src/focus_find_local.o \
	src/focus_command.o \
	src/focus.o \
	src/keybinds.o \
	src/err.o \
	src/draw.o

LT_PATH = lt/
LT_LIB = $(LT_PATH)/bin/lt.a

DEPS = $(patsubst %.o,%.deps,$(OBJS))

ifdef DEBUG
	CC_FLAGS += -fno-omit-frame-pointer -O0 -g
	LNK_FLAGS += -g -rdynamic
else
	CC_FLAGS += -O2
endif

CC = cc
CC_FLAGS += -fmax-errors=3 -I$(LT_PATH)/include/ -std=c11 -Wall -Werror -Wno-strict-aliasing -Wno-error=unused-variable -Wno-unused-function -Wno-pedantic

LNK = cc
LNK_FLAGS += -o $(OUT) 
LNK_LIBS += -lpthread -ldl -lm

all: $(OUT)

install: all
	cp $(OUT) /usr/local/bin/

$(LT_LIB):
	make -C $(LT_PATH)

$(OUT):	$(LT_LIB) $(OBJS)
	$(LNK) $(LNK_FLAGS) $(OBJS) $(LT_LIB) $(LNK_LIBS)

%.o: %.c makefile
	$(CC) $(CC_FLAGS) -MM -MT $@ -MF $(patsubst %.o,%.deps,$@) $<
	$(CC) $(CC_FLAGS) -c $< -o $@

-include $(DEPS)

clean:
	-rm $(OUT) $(OBJS) $(DEPS)

.PHONY: all install clean
