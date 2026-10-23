CXX ?= g++
CXXFLAGS ?= -std=c++17 -O2 -Wall -Wextra -pedantic
SRC_DIR = src
TEST_DIR = tests
BUILD_DIR = build

SRCS = $(SRC_DIR)/parser.cpp \
       $(SRC_DIR)/grid.cpp \
       $(SRC_DIR)/distance.cpp \
       $(SRC_DIR)/slots.cpp \
       $(SRC_DIR)/state.cpp \
       $(SRC_DIR)/commands.cpp \
       $(SRC_DIR)/formatter.cpp

OBJS = $(patsubst $(SRC_DIR)/%.cpp, $(BUILD_DIR)/%.o, $(SRCS))

TARGET = parking

TEST_SRCS = $(wildcard $(TEST_DIR)/test_*.cpp)
TEST_BINS = $(patsubst $(TEST_DIR)/%.cpp, $(BUILD_DIR)/%, $(TEST_SRCS))

.PHONY: all clean test sanitize

all: $(TARGET)

$(TARGET): $(OBJS) $(BUILD_DIR)/main.o
	$(CXX) $(CXXFLAGS) $^ -o $@

$(BUILD_DIR)/%.o: $(SRC_DIR)/%.cpp | $(BUILD_DIR)
	$(CXX) $(CXXFLAGS) -c $< -o $@

$(BUILD_DIR)/main.o: $(SRC_DIR)/main.cpp | $(BUILD_DIR)
	$(CXX) $(CXXFLAGS) -c $< -o $@

$(BUILD_DIR)/%: $(TEST_DIR)/%.cpp $(OBJS) | $(BUILD_DIR)
	$(CXX) $(CXXFLAGS) $< $(OBJS) -o $@

$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)

test: $(TEST_BINS)
	@echo "=== Running all tests ==="
	@for test_bin in $(TEST_BINS); do \
		echo "Running $$test_bin..."; \
		$$test_bin || exit 1; \
	done
	@echo "=== All tests passed successfully! ==="

sanitize: CXXFLAGS += -fsanitize=address,undefined -g
sanitize: clean test

clean:
	rm -rf $(BUILD_DIR) $(TARGET)
