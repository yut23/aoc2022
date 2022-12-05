CXX = clang++
LOCAL_CXXFLAGS = -Wall -O3 -std=c++20 -fsanitize=undefined -fsanitize=address #$(shell pkg-config --cflags $(libs_$(notdir $*)))
DEBUG_CXXFLAGS = $(LOCAL_CXXFLAGS) -g -Og -DDEBUG_MODE
LDFLAGS = -Wl,--as-needed #$(shell pkg-config --libs $(libs_$(notdir $*)))

EXECUTABLES := $(patsubst src/%.cpp,bin/%,$(wildcard src/day*.cpp))
DEBUG_EXECUTABLES := $(EXECUTABLES:%=%.debug)
all: $(EXECUTABLES) #$(DEBUG_EXECUTABLES)
debug: $(DEBUG_EXECUTABLES)

list:
	@printf 'normal: %s\n' $(EXECUTABLES)
	@printf 'debug:  %s\n' $(DEBUG_EXECUTABLES)

bin/%: src/%.cpp src/lib.h
	@mkdir -p bin
	$(CXX) $(LOCAL_CXXFLAGS) $(CXXFLAGS) $< -o $@ $(LDFLAGS)
bin/%.debug: src/%.cpp src/lib.h
	@mkdir -p bin
	$(CXX) $(DEBUG_CXXFLAGS) $(CXXFLAGS) $< -o $@ $(LDFLAGS)

clean:
	rm -f $(EXECUTABLES) $(DEBUG_EXECUTABLES)

.PHONY: all debug clean list
