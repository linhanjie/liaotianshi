include message_queue_test/Makefile

CC      = gcc
LINK    = gcc
CFLAGS  = -g -Wall -O2
TARGET  = server
#SRCS    += $(wildcard *.c)

C_OBJS  = $(patsubst %.c, %.o, $(wildcard *.c))

all:$(TARGET)

$(TARGET): $(C_OBJS)
	$(LINK) $(CFLAGS) -o $@ $^ -lpthread
%.o:%.c
	$(CC) $(CFLAGS) -c -o $@ $<

.PHONY:clean
clean:
	rm -rf *.o $(TARGET) $(CXX_OBJS) $(C_OBJS)


