CC      = gcc
LINK    = gcc
CFLAGS  = -g -Wall -O2
TARGET  = client 

C_OBJS = client.o

all:$(TARGET)

$(TARGET): $(C_OBJS)
	$(LINK) $(CFLAGS) -o $@ $^ -lpthread
%.o:%.c
	$(CC) $(CFLAGS) -c -o $@ $<

clean:
	rm -rf *.o $(TARGET) $(C_OBJS)

