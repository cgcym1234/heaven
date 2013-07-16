CFLAGS = -D_REENTRANT -g -Wall
DEBUG  =
LDFLAG =
CC = gcc

INC_PATH = 
LIB_PATH =  

LIBS = -lpthread

SRC  = fix_mem.c fix_test.c
OBJS = $(SRC:.c=.o)

TARGET = tpool

all: $(TARGET)

%.o: %.c
	$(CC) $(CFLAGS) $(INC_PATH) -c $?

$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) -o $(TARGET) $(OBJS) $(LIB_PATH) $(LIBS)

clean:
	rm -rf *.o
	rm -rf $(TARGET)

install: all
	cp $(TARGET) $(HOME)/bin/

rebuild: clean all
