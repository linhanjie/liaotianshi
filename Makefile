CC      = gcc
LINK    = gcc
CFLAGS  = -g -Wall -O2
TARGET  = server

C_OBJS  = $(patsubst %.c, %.o, $(wildcard *.c))

all:$(TARGET)

$(TARGET): $(C_OBJS)
	$(LINK) $(CFLAGS) -o $@ $^ -lpthread
	make -C message_queue_test/
	make -C hash_table_test/

%.o:%.c
	$(CC) $(CFLAGS) -c -o $@ $<

.PHONY:clean
clean:
	rm -rf *.o $(TARGET) $(C_OBJS)
	make clean -C message_queue_test/
	make clean -C hash_table_test/


