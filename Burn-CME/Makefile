CXX = g++
CXXFLAGS = -std=c++17 -Wall -Wextra -O2
LDFLAGS = 

TARGET = burn-cme
SRC_DIR = src/burn-cme
BUILD_DIR = build

SRCS = $(SRC_DIR)/main.cpp
OBJS = $(BUILD_DIR)/main.o

.PHONY: all clean run

all: $(BUILD_DIR) $(TARGET)

$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)

$(TARGET): $(OBJS)
	$(CXX) $(CXXFLAGS) -o $@ $^ $(LDFLAGS)
	@echo ""
	@echo "Build complete: $(TARGET)"
	@echo "Run with: ./$(TARGET)"

$(BUILD_DIR)/%.o: $(SRC_DIR)/%.cpp
	$(CXX) $(CXXFLAGS) -c -o $@ $<

clean:
	rm -rf $(BUILD_DIR) $(TARGET)

run: $(TARGET)
	./$(TARGET)

install: $(TARGET)
	install -m 755 $(TARGET) /usr/local/bin/

help:
	@echo "Burn-CME Makefile"
	@echo ""
	@echo "Targets:"
	@echo "  all     - Build the application (default)"
	@echo "  run     - Build and run the application"
	@echo "  clean   - Remove build files"
	@echo "  install - Install to /usr/local/bin"
	@echo "  help    - Show this help message"
