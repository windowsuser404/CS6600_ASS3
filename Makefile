# Set the compiler and compiler flags
CC = g++
OPT = -O3
WARN = -Wall
CFLAGS = $(OPT) $(WARN) -I$(INCLUDE_DIR) -g

# Directories
SRC_DIR = src
INCLUDE_DIR = include
BUILD_DIR = build

# Check if directories exist
$(shell mkdir -p $(SRC_DIR) $(INCLUDE_DIR) $(BUILD_DIR))

# List all .cpp files in src directory
SRC_FILES = $(wildcard $(SRC_DIR)/*.cpp)
# Generate corresponding .o files in the build directory
OBJ_FILES = $(patsubst $(SRC_DIR)/%.cpp, $(BUILD_DIR)/%.o, $(SRC_FILES))

# Header files dependencies
DEPS = $(wildcard $(INCLUDE_DIR)/*.h)

# Output executable name
TARGET = ooosim

# Default rule
all: $(TARGET)
	@echo "Build complete!"

run: $(TARGET)
	@echo "Running tests..."
	@if [ -f ./TEST.sh ]; then \
		./TEST.sh > ./run.log; \
	else \
		echo "Error: TEST.sh not found!"; \
		exit 1; \
	fi

# Rule for building the executable
$(TARGET): $(OBJ_FILES)
	$(CC) -o $@ $^ $(CFLAGS)

# Rule for compiling .cpp to .o, including header dependencies
$(BUILD_DIR)/%.o: $(SRC_DIR)/%.cpp $(DEPS)
	@mkdir -p $(BUILD_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

# Clean rule to remove all compiled files
clean:
	rm -rf $(BUILD_DIR) $(TARGET) run.log

# Help target
help:
	@echo "Available targets:"
	@echo "  all     : Build the project (default)"
	@echo "  run     : Build and run tests"
	@echo "  clean   : Remove build files"
	@echo "  help    : Show this help message"

# Phony targets
.PHONY: all clean help run
