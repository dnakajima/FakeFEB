TARGET=FakeFEB
CC=gcc
LDFLAGS=-std=gnu99
SRCS=$(wildcard ./*.c)
OBJS=$(SRCS:%.c=%.o)
all:$(TARGET)
$(TARGET): $(OBJS)
	$(CC) -o $@ $^ $(LDFLAGS)

%.o:%.c
	$(CC) -o $@ -c $^
clean: 
	$(RM) $(OBJS) $(TARGET)
