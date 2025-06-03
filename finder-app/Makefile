CROSS_COMPILE ?= 
CC := $(CROSS_COMPILE)gcc
CFLAGS = -Wall -Wextra -g
TARGET = writer
SRCS = writer.c
OBJS = $(SRCS:.c=.o)

# Default target 
all: $(TARGET)

$(TARGET): $(OBJS) 
		$(CC) $(CFLAGS) -o $@ $^

%.o	: %.c
		$(CC) $(CFLAGS) -c $< -O $@

clean	:
		rm -f $(OBJS) $(TARGET)

.PHONY: all clean
