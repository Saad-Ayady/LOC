# ====================================================
# Makefile for Windows Enumeration Tool
# Supports: Native Windows (gcc) and Linux Cross-compilation (mingw)
# ====================================================

# Auto-detect OS and set compiler
ifeq ($(OS),Windows_NT)
    # Native Windows
    CC = gcc
    TARGET_EXT = .exe
    RM = del /Q
    MKDIR = mkdir
    WINDRES = windres
    IS_WINDOWS = 1
else
    # Linux/Mac with cross-compilation
    CC = x86_64-w64-mingw32-gcc
    TARGET_EXT = .exe
    RM = rm -f
    MKDIR = mkdir -p
    WINDRES = x86_64-w64-mingw32-windres
    IS_WINDOWS = 0
endif

# Project info
PROJECT_NAME = mainLOC
VERSION = 1.0.0

# Directories
SRC_DIR = .
MODELS_DIR = src/models
ENUM_DIR = $(MODELS_DIR)/enum
COLOR_DIR = $(MODELS_DIR)/color
SEIMP_DIR = $(MODELS_DIR)/SeImperSona
USP_DIR = $(MODELS_DIR)/usp
DLL_DIR = $(MODELS_DIR)/dllHajacing
SERVICE_DIR = $(MODELS_DIR)/serviceMisconfiguration
BUILD_DIR = build
OBJ_DIR = $(BUILD_DIR)/obj
BIN_DIR = $(BUILD_DIR)/bin

# Include paths
INCLUDES = -I. \
           -Isrc \
           -I$(MODELS_DIR) \
           -I$(ENUM_DIR) \
           -I$(COLOR_DIR) \
           -I$(SEIMP_DIR) \
           -I$(USP_DIR) \
           -I$(DLL_DIR) \
           -I$(SERVICE_DIR)

# Source files
MAIN_SRC = loc-win.c

ENUM_SRCS = $(ENUM_DIR)/enum.c \
            $(ENUM_DIR)/src/UserAndHost.c \
            $(ENUM_DIR)/src/installApps.c \
            $(ENUM_DIR)/src/networkInfo.c \
            $(ENUM_DIR)/src/osInfo.c \
            $(ENUM_DIR)/src/prosesRun.c \
            $(ENUM_DIR)/src/userGrops.c

OTHER_SRCS = $(COLOR_DIR)/colorPrint.c \
             $(SEIMP_DIR)/seImperSona.c \
             $(USP_DIR)/getUspPaht.c \
             $(DLL_DIR)/dllCheck.c \
             $(SERVICE_DIR)/serMis.c

ALL_SRCS = $(MAIN_SRC) $(ENUM_SRCS) $(OTHER_SRCS)

# Object files (place in build directory)
OBJS = $(patsubst %.c,$(OBJ_DIR)/%.o,$(notdir $(ALL_SRCS)))

# Dep files for automatic dependencies
DEPS = $(OBJS:.o=.d)

# Target executables
TARGET = $(BIN_DIR)/$(PROJECT_NAME)$(TARGET_EXT)
TARGET_DEBUG = $(BIN_DIR)/$(PROJECT_NAME)-debug$(TARGET_EXT)
TARGET_RELEASE = $(BIN_DIR)/$(PROJECT_NAME)-release$(TARGET_EXT)

# Compiler flags
BASE_CFLAGS = -Wall -Wextra $(INCLUDES)
BASE_LDFLAGS = -ladvapi32 -lshlwapi -lnetapi32 -liphlpapi -lversion -lws2_32 -lole32

# Platform-specific flags
ifeq ($(IS_WINDOWS),1)
    # Native Windows flags
    CFLAGS = $(BASE_CFLAGS) -D_WIN32_WINNT=0x0600 -DWINDOWS_NATIVE
    LDFLAGS = $(BASE_LDFLAGS)
else
    # Cross-compilation flags
    CFLAGS = $(BASE_CFLAGS) -D_WIN32_WINNT=0x0600 -DCROSS_COMPILE
    LDFLAGS = $(BASE_LDFLAGS) -static-libgcc -static-libstdc++
endif

# Build configurations
DEBUG_CFLAGS = -g -DDEBUG -O0
RELEASE_CFLAGS = -O2 -s -DNDEBUG
STATIC_CFLAGS = -static

# ====================================================
# Build Rules
# ====================================================

# Default: release build
all: release

# Release build (optimized, stripped)
release: CFLAGS += $(RELEASE_CFLAGS)
release: $(TARGET_RELEASE)
	@echo "[+] Release build complete: $(TARGET_RELEASE)"

# Debug build (with symbols)
debug: CFLAGS += $(DEBUG_CFLAGS)
debug: $(TARGET_DEBUG)
	@echo "[+] Debug build complete: $(TARGET_DEBUG)"

# Static build (standalone executable)
static: CFLAGS += $(RELEASE_CFLAGS) $(STATIC_CFLAGS)
static: LDFLAGS += -static
static: $(BIN_DIR)/$(PROJECT_NAME)-static$(TARGET_EXT)
	@echo "[+] Static build complete: $(BIN_DIR)/$(PROJECT_NAME)-static$(TARGET_EXT)"

# Create release executable
$(TARGET_RELEASE): $(OBJS) | $(BIN_DIR)
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)
ifeq ($(IS_WINDOWS),0)
	@x86_64-w64-mingw32-strip $@ 2>/dev/null || true
endif

# Create debug executable
$(TARGET_DEBUG): $(OBJS) | $(BIN_DIR)
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

# Create static executable
$(BIN_DIR)/$(PROJECT_NAME)-static$(TARGET_EXT): $(OBJS) | $(BIN_DIR)
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS) -static
ifeq ($(IS_WINDOWS),0)
	@x86_64-w64-mingw32-strip $@ 2>/dev/null || true
endif

# Compile C files with dependency generation
$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c | $(OBJ_DIR)
	$(CC) $(CFLAGS) -MMD -c $< -o $@

$(OBJ_DIR)/%.o: $(ENUM_DIR)/%.c | $(OBJ_DIR)
	$(CC) $(CFLAGS) -MMD -c $< -o $@

$(OBJ_DIR)/%.o: $(ENUM_DIR)/src/%.c | $(OBJ_DIR)
	$(CC) $(CFLAGS) -MMD -c $< -o $@

$(OBJ_DIR)/%.o: $(COLOR_DIR)/%.c | $(OBJ_DIR)
	$(CC) $(CFLAGS) -MMD -c $< -o $@

$(OBJ_DIR)/%.o: $(SEIMP_DIR)/%.c | $(OBJ_DIR)
	$(CC) $(CFLAGS) -MMD -c $< -o $@

$(OBJ_DIR)/%.o: $(USP_DIR)/%.c | $(OBJ_DIR)
	$(CC) $(CFLAGS) -MMD -c $< -o $@

$(OBJ_DIR)/%.o: $(DLL_DIR)/%.c | $(OBJ_DIR)
	$(CC) $(CFLAGS) -MMD -c $< -o $@

$(OBJ_DIR)/%.o: $(SERVICE_DIR)/%.c | $(OBJ_DIR)
	$(CC) $(CFLAGS) -MMD -c $< -o $@

# Create directories
$(BUILD_DIR):
	$(MKDIR) $(BUILD_DIR)

$(OBJ_DIR): | $(BUILD_DIR)
	$(MKDIR) $(OBJ_DIR)

$(BIN_DIR): | $(BUILD_DIR)
	$(MKDIR) $(BIN_DIR)

# ====================================================
# Utility Rules
# ====================================================

# Clean build files
clean:
	$(RM) $(BUILD_DIR)
	@echo "[+] Cleaned build directory"

# Rebuild everything
rebuild: clean all

# Run on Windows (native) or with Wine (Linux)
run: $(TARGET_RELEASE)
ifeq ($(IS_WINDOWS),1)
	@echo "[*] Running on Windows Native..."
	@$(TARGET_RELEASE)
else
	@echo "[*] Running with Wine..."
	@wine $(TARGET_RELEASE) 2>/dev/null || \
	 echo "[-] Wine not found or error. Install with: sudo apt install wine"
endif

# Show build info
info:
	@echo "=== Build Information ==="
	@echo "Operating System: $(OS)"
	@echo "Compiler: $(CC)"
	@echo "Target: $(PROJECT_NAME)$(TARGET_EXT)"
	@echo "Source Files: $(words $(ALL_SRCS))"
	@echo "Build Mode: $(if $(findstring debug,$(MAKECMDGOALS)),Debug,Release)"
	@echo "Platform: $(if $(IS_WINDOWS),Windows Native,Linux Cross-compilation)"
	@echo "========================="

# List source files
list:
	@echo "Source files:"
	@for src in $(ALL_SRCS); do echo "  $$src"; done

# Check compiler availability
check:
	@echo "[*] Checking compiler..."
	@which $(CC) > /dev/null && \
	 echo "[+] Compiler found: $(CC)" || \
	 echo "[-] ERROR: Compiler not found!"
	@echo "[*] Checking dependencies..."
ifeq ($(IS_WINDOWS),0)
	@which wine > /dev/null && \
	 echo "[+] Wine found" || \
	 echo "[-] Wine not installed (optional for running)"
endif

# Create distribution package
dist: release
	@echo "[*] Creating distribution package..."
	$(MKDIR) dist
	cp $(TARGET_RELEASE) dist/
	cp README.md dist/ 2>/dev/null || true
	@echo "[+] Distribution created in 'dist/'"

# Help message
help:
	@echo "Available targets:"
	@echo "  all/release  - Build release version (default)"
	@echo "  debug        - Build debug version with symbols"
	@echo "  static       - Build static linked executable"
	@echo "  clean        - Remove all build files"
	@echo "  rebuild      - Clean and rebuild"
	@echo "  run          - Run executable (native or with Wine)"
	@echo "  info         - Show build information"
	@echo "  list         - List all source files"
	@echo "  check        - Check compiler and dependencies"
	@echo "  dist         - Create distribution package"
	@echo "  help         - Show this help"
	@echo ""
	@echo "Environment variables:"
	@echo "  CC           - Override compiler (e.g., CC=gcc make)"
	@echo ""
	@echo "Current settings:"
	@echo "  OS: $(OS)"
	@echo "  Compiler: $(CC)"
	@echo "  Target: $(PROJECT_NAME)$(TARGET_EXT)"

# Include dependency files
-include $(DEPS)

.PHONY: all release debug static clean rebuild run info list check dist help