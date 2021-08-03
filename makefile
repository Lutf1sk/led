
OUT = lutcreator

RUN_ARGS = test.txt

OBJS = \
	src/main.o \
	src/clr.o \
	src/doc.o \
	src/highlight.o \
	src/editor.o \
	src/custom_keys.o \
	src/file_browser.o \
	src/focus_editor.o \
	src/focus_error.o \
	src/focus_browse_files.o \
	src/focus_browse_filesystem.o \
	src/focus_goto.o \
	src/focus_find_local.o \
	src/focus.o \
	src/fhl.o \
	src/err.o \
	src/allocators.o \
	src/pool.o \
	src/conf.o \
	src/assert.o

LNK_LIBS = -lncurses

CC_FLAGS = -O2

ifdef DEBUG
	CC_FLAGS += -rdynamic -g
	LNK_FLAGS += -rdynamic -g
endif

# ----------==========

CC = cc
LNK = cc

DEPS = $(patsubst %.o,%.d,$(OBJS))

CC_FLAGS += -Wall -I./
LNK_FLAGS +=

OUT_PATH = bin/$(OUT)

all: $(OUT_PATH)

.PHONY: install
install: all
	cp $(OUT_PATH) /usr/local/bin/$(OUT)

LNK_CMD = $(LNK) $(LNK_FLAGS) -o $(OUT_PATH) $(OBJS) $(LNK_LIBS)
$(OUT_PATH): $(OBJS)
	@-mkdir -p bin
	@printf "Linking %-25s (%s)\n" "$(OUT)" "$(LNK_CMD)"
	@$(LNK_CMD)

run: all
	$(OUT_PATH) $(RUN_ARGS)

.PHONY: clean all run analyze

clean:
	-rm $(OBJS) $(DEPS) $(OUT_PATH)

analyze:
	@clang-check --analyze $(patsubst %.o,%.c,$(OBJS))

%.o: %.c
	@printf "Compiling %-25s (%s)\n" "$<" "$(CC) -c $< -o $@ $(CC_FLAGS)"
	@$(CC) -MM -MT $@ -MF $(patsubst %.o,%.d,$@) $< $(CC_FLAGS)
	@$(CC) -c $< -o $@ $(CC_FLAGS)

-include $(DEPS)


