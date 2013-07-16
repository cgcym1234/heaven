CFLAGS = -O2
DEBUG  =
LDFLAG =

INC_PATH = 
LIB_PATH = 

LIBS = -lpthread

SRC  = thread_pool.c thread_pool_test.c
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

