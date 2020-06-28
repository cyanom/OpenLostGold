TARGET_EXEC ?= openlostgold

BUILD_DIR ?= ./build
SRC_DIRS ?= ./src

SRCS := $(shell find $(SRC_DIRS) -name *.cpp -or -name *.c -or -name *.s)
OBJS := $(SRCS:%=$(BUILD_DIR)/%.o)
DEPS := $(OBJS:.o=.d)

INC_DIRS := $(shell find $(SRC_DIRS) -type d)
INC_FLAGS := $(addprefix -I,$(INC_DIRS))

STB_INCLUDE_PATH = ./lib/stb/
RAPIDXML_INCLUDE_PATH = ./lib/rapidxml-1.13/
LDFLAGS = -lvulkan -lglfw -lm -lstdc++ -lstdc++fs
CPPFLAGS ?= $(INC_FLAGS) -g -std=c++17 -Wno-return-type -I$(STB_INCLUDE_PATH) -I$(RAPIDXML_INCLUDE_PATH) -MMD -MP

$(BUILD_DIR)/$(TARGET_EXEC): $(OBJS)
	$(CC) $(OBJS) -o $@ $(LDFLAGS)

# assembly
$(BUILD_DIR)/%.s.o: %.s
	$(MKDIR_P) $(dir $@)
	$(AS) $(ASFLAGS) -c $< -o $@

# c source
$(BUILD_DIR)/%.c.o: %.c
	$(MKDIR_P) $(dir $@)
	$(CC) $(CPPFLAGS) $(CFLAGS) -c $< -o $@

# c++ source
$(BUILD_DIR)/%.cpp.o: %.cpp
	$(MKDIR_P) $(dir $@)
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) -c $< -o $@


.PHONY: clean

memory: $(BUILD_DIR)/$(TARGET_EXEC)
	valgrind ./$(BUILD_DIR)/$(TARGET_EXEC)

memory_full: $(BUILD_DIR)/$(TARGET_EXEC)
	valgrind --leak-check=full ./$(BUILD_DIR)/$(TARGET_EXEC)

memory_fullest: $(BUILD_DIR)/$(TARGET_EXEC)
	valgrind --leak-check=full --show-leak-kinds=all ./$(BUILD_DIR)/$(TARGET_EXEC)

test: $(BUILD_DIR)/$(TARGET_EXEC)
	./$(BUILD_DIR)/$(TARGET_EXEC)

clean:
	$(RM) -r $(BUILD_DIR)

-include $(DEPS)

MKDIR_P ?= mkdir -p

