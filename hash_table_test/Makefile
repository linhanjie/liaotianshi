CC      = gcc
LINK    = gcc
CFLAGS  = -g -Wall -O2
TARGET  = hash_table_test

C_OBJS = ../hash_table.o hash_table_test.o

all:$(TARGET)

$(TARGET): $(C_OBJS)
	$(LINK) $(CFLAGS) -o $@ $^ -lpthread
%.o:%.c
	$(CC) $(CFLAGS) -c -o $@ $<

clean:
	rm -rf *.o $(TARGET) $(C_OBJS)

