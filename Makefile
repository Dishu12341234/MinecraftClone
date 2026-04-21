# =========================
# Compiler
# =========================
CXX := g++
UNAME_S := $(shell uname -s)

# =========================
# Directories
# =========================
BUILD_DIR := build
SRC_DIR   := .

# =========================
# Sources
# =========================
SRCS := $(shell ls -S $(SRC_DIR)/*.cxx 2>/dev/null)
OBJS := $(patsubst %.cxx,$(BUILD_DIR)/%.o,$(notdir $(SRCS)))

# =========================
# Executables
# =========================
MAC_EXEC   := $(BUILD_DIR)/main_mac
LINUX_EXEC := $(BUILD_DIR)/main_linux

# =========================
# Vulkan SDK
# =========================
MAC_VULKAN_SDK   := /home/divyansh/SDKs/vksdk/1.4.335.0/
LINUX_VULKAN_SDK := /home/divyansh/SDKs/vksdk/1.4.335.0/x86_64

# =========================
# Flags
# =========================
COMMON_CXXFLAGS := -std=c++2b -Wall -g -O1 \
	-MMD -MP \
	-lprofiler \
	-I/usr/include \
	-fno-inline -rdynamic

MAC_CXXFLAGS := $(COMMON_CXXFLAGS) \
	-I$(MAC_VULKAN_SDK)/macOS/include

LINUX_CXXFLAGS := $(COMMON_CXXFLAGS) \
	-I$(LINUX_VULKAN_SDK)/include

COMMON_LDFLAGS := -L/opt/homebrew/lib -L./lib -lglfw -lfmt

MAC_LDFLAGS := $(COMMON_LDFLAGS) \
	-L$(MAC_VULKAN_SDK)/macOS/lib \
	-lvulkan -lMoltenVK \
	-framework Cocoa \
	-framework IOKit \
	-framework CoreFoundation \
	-framework QuartzCore \
	-framework Metal \
	-Wl,-rpath,$(MAC_VULKAN_SDK)/macOS/lib

LINUX_LDFLAGS := $(COMMON_LDFLAGS) -L$(LINUX_VULKAN_SDK)/lib -lvulkan

# =========================
# Colors
# =========================
YELLOW := \033[1;33m
GREEN  := \033[1;32m
RED    := \033[1;31m
BGMAGENTA := \033[45m
RESET  := \033[0m

# =========================
# Targets
# =========================
.PHONY: all clean

all: $(BUILD_DIR)
ifeq ($(UNAME_S),Darwin)
	@echo -e "$(YELLOW)Building for macOS...$(RESET)"
	$(MAKE) -j6 $(MAC_EXEC)
	@echo -e "$(GREEN)Running $(BGMAGENTA)$(MAC_EXEC)$(RESET)"
	./$(MAC_EXEC)
else
	@echo -e "$(YELLOW)Building for Linux...$(RESET)"
	$(MAKE) -j6 $(LINUX_EXEC)
endif

# =========================
# Build rules
# =========================
$(MAC_EXEC): $(OBJS)
	@echo -e "$(GREEN)Linking $(BGMAGENTA)$@$(RESET)"
	$(CXX) $(OBJS) $(MAC_CXXFLAGS) $(MAC_LDFLAGS) -o $@

$(LINUX_EXEC): $(OBJS)
	@printf "$(GREEN)Linking $(BGMAGENTA)$@$(RESET)\n"
	@$(CXX) $(OBJS) $(LINUX_CXXFLAGS) $(LINUX_LDFLAGS) -o $@
	@TOTAL_SRC=$$(find $(SRC_DIR) -name '*.cxx' | wc -l); \
	TOTAL_OBJ=$$(find $(BUILD_DIR) -name '*.o' | wc -l); \
	SRC_BYTES=$$(find $(SRC_DIR) -name '*.cxx' -exec wc -c {} + | tail -1 | awk '{print $$1}'); \
	OBJ_BYTES=$$(find $(BUILD_DIR) -name '*.o' -exec wc -c {} + | tail -1 | awk '{print $$1}'); \
	EXEC_BYTES=$$(wc -c < $@); \
	printf "\n"; \
	printf "$(GREEN)╔════════════════════════════════════════════════╗$(RESET)\n"; \
	printf "$(GREEN)║           Build Summary             		 ║$(RESET)\n"; \
	printf "$(GREEN)╠════════════════════════════════════════════════╣$(RESET)\n"; \
	printf "$(GREEN)║$(RESET)  📁 Sources compiled : $$TOTAL_SRC files\n"; \
	printf "$(GREEN)║$(RESET)  📦 Objects produced : $$TOTAL_OBJ files\n"; \
	printf "$(GREEN)║$(RESET)  📄 Total source size : $$SRC_BYTES bytes\n"; \
	printf "$(GREEN)║$(RESET)  🧱 Total object size : $$OBJ_BYTES bytes\n"; \
	printf "$(GREEN)║$(RESET)  🚀 Executable size   : $$EXEC_BYTES bytes\n"; \
	printf "$(GREEN)║$(RESET)  🎯 Output            : $@\n"; \
	printf "$(GREEN)╚════════════════════════════════════════════════╝$(RESET)\n"; \
	printf "\n"

# Object compilation
$(BUILD_DIR)/%.o: %.cxx
	@mkdir -p $(BUILD_DIR)
	@echo -e "$(YELLOW)Compiling $(BGMAGENTA)$<$(RESET)"
ifeq ($(UNAME_S),Darwin)
	@START=$$(date +%s%3N); \
	$(CXX) $(MAC_CXXFLAGS) -c $< -o $@; \
	END=$$(date +%s%3N); \
	SIZE_IN=$$(wc -c < $<); \
	SIZE_OUT=$$(wc -c < $@); \
	echo -e "  $(GREEN)✔  $<$(RESET)"; \
	echo -e "     ⏱  Time   : $$((END - START))ms"; \
	echo -e "     📄 Source : $$SIZE_IN bytes"; \
	echo -e "     📦 Object : $$SIZE_OUT bytes"; \
	echo ""
else
	@START=$$(date +%s%3N); \
	$(CXX) $(LINUX_CXXFLAGS) -c $< -o $@; \
	END=$$(date +%s%3N); \
	SIZE_IN=$$(wc -c < $<); \
	SIZE_OUT=$$(wc -c < $@); \
	echo -e "  $(GREEN)✔  $<$(RESET)"; \
	echo -e "     ⏱  Time   : $$((END - START))ms"; \
	echo -e "     📄 Source : $$SIZE_IN bytes"; \
	echo -e "     📦 Object : $$SIZE_OUT bytes"; \
	echo ""
endif

-include $(BUILD_DIR)/*.d

# =========================
# Clean
# =========================
clean:
	@echo -e "$(RED)Cleaning build directory...$(RESET)"
	rm -rf $(BUILD_DIR)
	mkdir -p $(BUILD_DIR)
