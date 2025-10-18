CC := clang
DBG_BIN := lldb
# CFLAGS = -D_GNU_SOURCE
CFLAGS += -std=c11
CFLAGS += -std=c11
CFLAGS += -Wall
CFLAGS += -Wextra
CFLAGS += -pedantic
# CFLAGS += -Werror
CFLAGS += -Wmissing-declarations
CFLAGS += $(shell pkg-config --cflags sdl3 sdl3-image)
LDFLAGS := $(shell pkg-config --libs sdl3 sdl3-image) -lm
ASANFLAGS := -fsanitize=address -fno-common -fno-omit-frame-pointer

SRC_FILES := ./src/*.c 
BIN_DIR := ./bin
BIN := $(BIN_DIR)/food-fight

build: bin-dir
	$(CC) $(CFLAGS) $(LIBS) $(SRC_FILES) -o $(BIN) $(LDFLAGS)

bin-dir:
	mkdir -p $(BIN_DIR)

run: build
	@$(BIN) $(ARGS)

run-hud: build
	LD_PRELOAD=/usr/lib/mangohud/libMangoHud_dlsym.so mangohud $(BIN) $(ARGS)

debug: debug-build
	# LSAN_OPTIONS=verbosity=1:log_threads=1
	$(DBG_BIN) $(BIN) $(ARGS)

debug-build: bin-dir
	$(CC) $(ASANFLAGS) $(CFLAGS) -g $(LIBS) $(SRC_FILES) -o $(BIN) $(LDFLAGS)

debug-run: build
	@$(BIN) $(ARGS)

clean:
	rm -rf $(BIN_DIR)/*

gen-compilation-db:
	bear -- make build

