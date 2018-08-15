TARGET = parse_fat_img

#CROSS_COMPILE = arm-linux-gnueabihf-

CFLAGS = -g  -Wall  -pthread 
LDFLAGS =  -Wall  -lpthread -lm
CC = $(CROSS_COMPILE)gcc
#ARCH= arm


OBJS =  main.o \
		fat32.o

all: $(TARGET)
$(TARGET): $(OBJS)
	$(CC) $(LDFLAGS)   $^ -o $@  
%.o : %.c
	$(CC) $(CFLAGS) -c $< -o $@

.PHONY: clean
clean:
	rm -f $(TARGET)  *.o *~ 
