CC ?= cc
CPPFLAGS += -Iinclude -D_NETBSD_SOURCE
CFLAGS += -Wall -Wextra -std=c99 -Iinclude -D_NETBSD_SOURCE
TARGET = ls_custom
BUILD_DIR = build
SRC_DIR = src

SRCS = $(SRC_DIR)/main.c \
       $(SRC_DIR)/parse_args.c \
       $(SRC_DIR)/traverse.c \
       $(SRC_DIR)/sort.c \
       $(SRC_DIR)/display.c \
       $(SRC_DIR)/utils.c

OBJS = $(BUILD_DIR)/main.o \
       $(BUILD_DIR)/parse_args.o \
       $(BUILD_DIR)/traverse.o \
       $(BUILD_DIR)/sort.o \
       $(BUILD_DIR)/display.o \
       $(BUILD_DIR)/utils.o

all: $(BUILD_DIR) $(TARGET)

$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $(OBJS)

$(BUILD_DIR)/main.o: $(SRC_DIR)/main.c
	@mkdir -p $(BUILD_DIR)
	$(CC) $(CFLAGS) $(CPPFLAGS) -c $(SRC_DIR)/main.c -o $@

$(BUILD_DIR)/parse_args.o: $(SRC_DIR)/parse_args.c
	@mkdir -p $(BUILD_DIR)
	$(CC) $(CFLAGS) $(CPPFLAGS) -c $(SRC_DIR)/parse_args.c -o $@

$(BUILD_DIR)/traverse.o: $(SRC_DIR)/traverse.c
	@mkdir -p $(BUILD_DIR)
	$(CC) $(CFLAGS) $(CPPFLAGS) -c $(SRC_DIR)/traverse.c -o $@

$(BUILD_DIR)/sort.o: $(SRC_DIR)/sort.c
	@mkdir -p $(BUILD_DIR)
	$(CC) $(CFLAGS) $(CPPFLAGS) -c $(SRC_DIR)/sort.c -o $@

$(BUILD_DIR)/display.o: $(SRC_DIR)/display.c
	@mkdir -p $(BUILD_DIR)
	$(CC) $(CFLAGS) $(CPPFLAGS) -c $(SRC_DIR)/display.c -o $@

$(BUILD_DIR)/utils.o: $(SRC_DIR)/utils.c
	@mkdir -p $(BUILD_DIR)
	$(CC) $(CFLAGS) $(CPPFLAGS) -c $(SRC_DIR)/utils.c -o $@

$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)

clean:
	rm -rf $(BUILD_DIR) $(TARGET) $(TARGET).exe

.PHONY: all clean
