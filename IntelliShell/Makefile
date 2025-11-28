CXX = g++
# Try C++14 if C++17 is not supported (for older g++ versions)
CXXFLAGS = -Wall -Wextra -std=c++11 -pthread -D_REENTRANT -g -O2 -DNO_STD_THREAD
# Check if ncurses is available, if not, build without UI features
NCURSES_LIB = -lncurses
ifeq ($(OS),Windows_NT)
    LIBS = -lpsapi
else
    LIBS = -lpthread
endif
TARGET = smart_shell
SRCDIR = src
INCDIR = include
OBJDIR = obj

# Source files
SOURCES = $(wildcard $(SRCDIR)/*.cpp)
OBJECTS = $(SOURCES:$(SRCDIR)/%.cpp=$(OBJDIR)/%.o)

# Default target
all: $(TARGET)

# Create object directory
$(OBJDIR):
	@if not exist $(OBJDIR) mkdir $(OBJDIR)

# Build the main executable
$(TARGET): $(OBJDIR) $(OBJECTS)
	@echo "Linking..."
	$(CXX) $(OBJECTS) -o $(TARGET) $(LIBS)

# Compile source files
$(OBJDIR)/%.o: $(SRCDIR)/%.cpp
	$(CXX) $(CXXFLAGS) -I$(INCDIR) -c $< -o $@

# Clean build artifacts
clean:
	@if exist $(OBJDIR) rmdir /s /q $(OBJDIR) 2>nul || rm -rf $(OBJDIR)
	@if exist $(TARGET) del /q $(TARGET) 2>nul || rm -f $(TARGET)
	@if exist $(TARGET).exe del /q $(TARGET).exe 2>nul || rm -f $(TARGET).exe

# Install dependencies (Ubuntu/Debian)
install-deps:
	sudo apt-get update
	sudo apt-get install -y build-essential libncurses5-dev

# Run the shell
run: $(TARGET)
	./$(TARGET)

# Debug build
debug: CXXFLAGS += -DDEBUG -g3
debug: $(TARGET)

.PHONY: all clean install-deps run debug
