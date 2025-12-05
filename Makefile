# ===========================
# Makefile for mainLOC.exe (Windows Local Enumeration Tool)
# ===========================

# Compiler (MinGW-w64 cross-compiler for Windows)
CC := x86_64-w64-mingw32-gcc

# Output executable
TARGET := mainLOC.exe

# Compiler flags
CFLAGS := -Wall -O2 -std=c11

# Linker flags and libraries
LDFLAGS := 
LIBS    := -ladvapi32 -lshlwapi -lnetapi32 -liphlpapi -lversion -lws2_32 -lole32

# Source files - fixed paths and proper continuation
SRC := loc-win.c \
       args.c \
       src/models/enum/enum.c \
       src/models/enum/src/UserAndHost.c \
       src/models/enum/src/installApps.c \
       src/models/enum/src/networkInfo.c \
       src/models/enum/src/osInfo.c \
       src/models/enum/src/prosesRun.c \
       src/models/enum/src/userGrops.c \
       src/models/color/colorPrint.c \
       src/models/SeImperSona/seImperSona.c \
       src/models/usp/getUspPaht.c \
       src/models/dllHajacing/dllCheck.c \
       src/models/serviceMisconfiguration/serMis.c \
       src/models/privescCveCheck/fullScanCVE.c \
       src/models/privescCveCheck/cve/CVE-2021-36934.c \
       src/models/privescCveCheck/cve/CVE-2024-21338.c

# Object files (automatically derived from sources)
OBJ := $(SRC:.c=.o)

# ===========================
# Build rules
# ===========================

# Default target
all: $(TARGET)

# Link object files into final executable
$(TARGET): $(OBJ)
	$(CC) $(CFLAGS) $(LDFLAGS) $(OBJ) -o $@ $(LIBS)

# Compile .c files to .o files
%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

# ===========================
# Utility targets
# ===========================

clean:
	rm -f $(TARGET) $(OBJ)

distclean: clean
	rm -f $(TARGET)

.PHONY: all clean distclean

# ===========================
# Optional: Show info
# ===========================

info:
	@echo "Target: $(TARGET)"
	@echo "Sources: $(SRC)"
	@echo "Objects: $(OBJ)"