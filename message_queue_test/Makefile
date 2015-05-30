CC      = gcc
LINK    = gcc
CFLAGS  = -g -Wall -O2
TARGET  = message_queue_test

C_OBJS = ../message_queue.c message_queue_test.c

all:$(TARGET)

$(TARGET): $(C_OBJS)
	$(LINK) $(CFLAGS) -o $@ $^ -lpthread
%.o:%.c
	$(CC) $(CFLAGS) -c -o $@ $<

.PHONY:clean
clean:
	rm -rf *.o $(TARGET) $(CXX_OBJS) $(C_OBJS)


