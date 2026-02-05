TARGET_EXEC := Gluon

BUILD_ROOT_DIR := ./build
SRC_DIR := ./src

SRCS := $(wildcard $(SRC_DIR)/*.cpp)

BUILD_CONFIG ?= release

WARN_FLAGS := -Wall -Wextra -Wpedantic -Wshadow -Wconversion -Wsign-conversion -Wcast-qual -Wold-style-cast -Wmissing-declarations -Wredundant-decls -Wextra-semi -Wnull-dereference -Wdouble-promotion -Wuseless-cast -Wlogical-op -Wduplicated-cond -Wduplicated-branches -Wundef -Wnon-virtual-dtor -Woverloaded-virtual -Wsuggest-override -Wzero-as-null-pointer-constant -Wcast-align=strict -Wformat=2 -Wimplicit-fallthrough=5 -Wunused-macros -Wshift-overflow=2 -Wctor-dtor-privacy -Werror

ifeq ($(BUILD_CONFIG), debug)
	CPP_FLAGS := -g -O0 -DDEBUG $(WARN_FLAGS) -std=c++23 -pthread
	LINK_FLAGS := -pthread
	BUILD_DIR := ./build/debug
else
	CPP_FLAGS := -O3 -DNDEBUG $(WARN_FLAGS) -std=c++23 -pthread
	LINK_FLAGS := -pthread
	BUILD_DIR := ./build/release
endif

OBJS := $(patsubst $(SRC_DIR)/%.cpp,$(BUILD_DIR)/%.o,$(SRCS))
DEPS := $(OBJS:.o=.d)

$(BUILD_DIR)/$(TARGET_EXEC): $(OBJS)
	$(CXX) $(OBJS) $(LINK_FLAGS) -o $@

$(BUILD_DIR)/%.o: $(SRC_DIR)/%.cpp
	mkdir -p $(dir $@)
	$(CXX) $(CPP_FLAGS) -MMD -MP -c $< -o $@

-include $(DEPS)

.PHONY: clean
clean:
	rm -rf $(BUILD_ROOT_DIR)