CFLAGS = -O2
DEBUG  =
LDFLAG =
CC = gcc -g -Wall

INC_PATH = -I../include
LIB_PATH = -L../lib

LIBS = -lpthread

SRC  = log.c log_test.c thread_pool.c
OBJS = $(SRC:.c=.o)

TARGET = test

all: $(TARGET)

$(SRC):
	@touch $@

%.o: %.c
	$(CC) $(CFLAGS) $(DEBUG) $(INC_PATH) -c $?

$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) $(LDFLAG) -o $(TARGET) $(OBJS) $(LIB_PATH) $(LIBS)

clean:
	rm -rf *.o
	rm -rf $(TARGET)

install: all
	cp $(TARGET) $(HOME)/bin/

rebuild: clean all

