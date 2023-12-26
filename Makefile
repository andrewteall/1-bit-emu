LIB_DIR = lib/
BIN_DIR = bin/

ALL_PROJECTS = ulog basm bemu

.PHONY: all clean prog $(ALL_PROJECTS) ulog-release
all: $(ALL_PROJECTS) prog

$(ALL_PROJECTS):
	$(MAKE) -C $@

bemu: ulog
basm: ulog

ulog-release:
	$(MAKE) -C ulog $@

prog: basm
	./bin/basm ./basm/asm/prog.asm

bemu-clean:
	$(MAKE) -C bemu clean

clean:
	if [ -d $(LIB_DIR) ]; then rm -r $(LIB_DIR); fi
	if [ -d $(BIN_DIR) ]; then rm -r $(BIN_DIR); fi
