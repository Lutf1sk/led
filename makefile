OUT = led

OBJS = \
	src/main.o \
	src/clr.o \
	src/doc.o \
	src/hl.o \
	src/hl_c.o \
	src/editor.o \
	src/file_browser.o \
	src/focus_terminal.o \
	src/focus_editor.o \
	src/focus_error.o \
	src/focus_browse_files.o \
	src/focus_browse_filesystem.o \
	src/focus_goto.o \
	src/focus_exit.o \
	src/focus_find_local.o \
	src/focus.o \
	src/err.o \
	src/draw.o

LT_PATH = lt/
LT_LIB = $(LT_PATH)/bin/lt.a

DEPS = $(patsubst %.o,%.deps,$(OBJS))

CC = cc
CC_FLAGS += -g -O2 -fmax-errors=3 -I$(LT_PATH)/include/ -std=c11 -Wall -Werror -Wno-strict-aliasing -Wno-error=unused-variable -Wno-unused-function -Wno-pedantic

LNK = cc
LNK_FLAGS += -o $(OUT) -g -rdynamic
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
