CC      = gcc
LINK    = gcc
CFLAGS  = -g -Wall -O2
TARGET  = message_queue_test

C_OBJS = ../message_queue.o ../log.o message_queue_test.o

all:$(TARGET)

$(TARGET): $(C_OBJS)
	$(LINK) $(CFLAGS) -o $@ $^ -lpthread
%.o:%.c
	$(CC) $(CFLAGS) -c -o $@ $<

clean:
	rm -rf *.o $(TARGET) $(C_OBJS)

