CXX = clang++
LOCAL_CXXFLAGS = -Wall -O3 -std=c++20 -fsanitize=undefined -fsanitize=address #$(shell pkg-config --cflags $(libs_$(notdir $*)))
DEBUG_CXXFLAGS = $(LOCAL_CXXFLAGS) -g -Og -DDEBUG_MODE
LDFLAGS = -Wl,--as-needed #$(shell pkg-config --libs $(libs_$(notdir $*)))
BEAR_ARGS = --config bear_config.json

EXECUTABLES := $(patsubst src/%.cpp,bin/%,$(wildcard src/day*.cpp))
DEBUG_EXECUTABLES := $(EXECUTABLES:%=%.debug)
all: $(EXECUTABLES) compile_commands.json #$(DEBUG_EXECUTABLES)
debug: $(DEBUG_EXECUTABLES)

list:
	@printf 'normal: %s\n' $(EXECUTABLES)
	@printf 'debug:  %s\n' $(DEBUG_EXECUTABLES)

compile_commands.json: Makefile
	@echo "Makefile changed, rebuilding entire compilation database..."
	bear $(BEAR_ARGS) -- make -B $(EXECUTABLES) $(DEBUG_EXECUTABLES)

bin/%: src/%.cpp src/lib.h
	@mkdir -p bin
	bear --append $(BEAR_ARGS) -- $(CXX) $(LOCAL_CXXFLAGS) $(CXXFLAGS) $< -o $@ $(LDFLAGS)
bin/%.debug: src/%.cpp src/lib.h
	@mkdir -p bin
	bear --append $(BEAR_ARGS) -- $(CXX) $(DEBUG_CXXFLAGS) $(CXXFLAGS) $< -o $@ $(LDFLAGS)

clean:
	rm -f $(EXECUTABLES) $(DEBUG_EXECUTABLES)

.PHONY: all debug clean list
