CC = clang
DBG_BIN = lldb
CFLAGS += -std=c11
CFLAGS += -Wall -pedantic -Wextra -Wmissing-declarations -Wfloat-conversion -Wsign-conversion -Wconversion
CFLAGS += -Werror -isystem ./lib
LIBS = -I./lib
CFLAGS += -I$$HOME/repos/3rd-party/raylib/src 
LDFLAGS += -L$$HOME/repos/3rd-party/raylib/build
LDFLAGS += -lraylib -lm -lpthread -ldl 
ASANFLAGS = -fsanitize=address -fno-common -fno-omit-frame-pointer

SRC_FILES = ./src/*.c
BIN_DIR = ./bin
BIN = $(BIN_DIR)/foodfight

build: bin-dir
	$(CC) $(CFLAGS) $(LIBS) $(SRC_FILES) -o $(BIN) $(LDFLAGS)

build-asan:
	$(CC) $(ASANFLAGS) $(CFLAGS) $(LIBS) $(SRC_FILES) -o $(BIN) $(LDFLAGS)

run: build
	@$(BIN) $(ARGS)

bin-dir:
	mkdir -p $(BIN_DIR)

debug: debug-build
	# LSAN_OPTIONS=verbosity=1:log_threads=1
	$(DBG_BIN) $(BIN) $(ARGS)

debug-build: bin-dir
	$(CC) $(ASANFLAGS) $(CFLAGS) -g $(LIBS) $(SRC_FILES) -o $(BIN) $(LDFLAGS)

debug-run: debug-build
	$(BIN) $(ARGS)

run-hud: build
	LD_PRELOAD=/usr/lib/mangohud/libMangoHud_dlsym.so mangohud $(BIN) $(ARGS)

clean:
	rm -rf $(BIN_DIR)/* 

gen-compilation-db:
	bear -- make build
