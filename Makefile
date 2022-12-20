ifeq ($(origin CXX), default)
	CXX = clang++
endif
LOCAL_CXXFLAGS = -Wall -Wextra -O3 -std=c++20 -fsanitize=undefined -fsanitize=address -fno-omit-frame-pointer #$(shell pkg-config --cflags $(libs_$(notdir $*)))
DEBUG_CXXFLAGS = $(LOCAL_CXXFLAGS) -g -Og -DDEBUG_MODE
FAST_CXXFLAGS = -Wall -Wextra -O3 -std=c++20 -fno-omit-frame-pointer
LDFLAGS = -Wl,--as-needed -fsanitize=undefined -fsanitize=address -fno-omit-frame-pointer #$(shell pkg-config --libs $(libs_$(notdir $*)))
FAST_LDFLAGS = -Wl,--as-needed -fno-omit-frame-pointer
BEAR_ARGS = --config bear_config.json

REL_BUILD_DIR = build/release
DEBUG_BUILD_DIR = build/debug
FAST_BUILD_DIR = build/fast

# List of all .cpp source files.
CPP = $(wildcard src/day*.cpp)

# All .o files go to build dirs
REL_OBJ = $(CPP:src/%.cpp=$(REL_BUILD_DIR)/%.o)
DEBUG_OBJ = $(CPP:src/%.cpp=$(DEBUG_BUILD_DIR)/%.o)
FAST_OBJ = $(CPP:src/%.cpp=$(FAST_BUILD_DIR)/%.o)
# gcc/clang will create these .d files containing dependencies.
DEP = $(REL_OBJ:.o=.d) $(DEBUG_OBJ:.o=.d) $(FAST_OBJ:.o=.d)

REL_EXECUTABLES := $(REL_OBJ:.o=)
DEBUG_EXECUTABLES := $(DEBUG_OBJ:.o=)
FAST_EXECUTABLES := $(FAST_OBJ:.o=)
all: compile_commands.json release debug fast
release: $(REL_EXECUTABLES)
debug: $(DEBUG_EXECUTABLES)
fast: $(FAST_EXECUTABLES)

list:
	@printf 'normal: %s\n' $(REL_EXECUTABLES)
	@printf 'debug:  %s\n' $(DEBUG_EXECUTABLES)
	@printf 'fast:   %s\n' $(FAST_EXECUTABLES)

compile_commands.json: Makefile
	@echo "Makefile changed, rebuilding entire compilation database..."
	rm -f compile_commands.json
	$(MAKE) -B $(REL_EXECUTABLES) $(DEBUG_EXECUTABLES)

# create build directories if they don't exist
$(REL_BUILD_DIR) $(DEBUG_BUILD_DIR) $(FAST_BUILD_DIR):
	mkdir -p $@

# Include all .d files
-include $(DEP)

# Build target for every single object file.
# The potential dependency on header files is covered
# by calling `-include $(DEP)`.
# The -MMD flags additionally creates a .d file with
# the same name as the .o file.
# Also include order-only dependencies on the build directory
$(REL_BUILD_DIR)/%.o: src/%.cpp | $(REL_BUILD_DIR)
	bear --append $(BEAR_ARGS) -- $(CXX) $(LOCAL_CXXFLAGS) $(CXXFLAGS) -MMD -c $< -o $@
$(DEBUG_BUILD_DIR)/%.o: src/%.cpp | $(DEBUG_BUILD_DIR)
	bear --append $(BEAR_ARGS) -- $(CXX) $(DEBUG_CXXFLAGS) $(CXXFLAGS) -MMD -c $< -o $@
$(FAST_BUILD_DIR)/%.o: src/%.cpp | $(FAST_BUILD_DIR)
	$(CXX) $(FAST_CXXFLAGS) $(CXXFLAGS) -MMD -c $< -o $@

# Link the object files into executables
$(REL_EXECUTABLES) $(DEBUG_EXECUTABLES): %: %.o
	$(CXX) $^ -o $@ $(LDFLAGS)
$(FAST_EXECUTABLES): %: %.o
	$(CXX) $^ -o $@ $(FAST_LDFLAGS)

clean:
	rm -f $(REL_EXECUTABLES) $(DEBUG_EXECUTABLES) $(FAST_EXECUTABLES) $(REL_OBJ) $(DEBUG_OBJ) $(FAST_OBJ) $(DEP)

.PHONY: all debug clean list
