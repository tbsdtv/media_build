CC=gcc

SRC=femon-s2.c
OBJ=femon-s2.o

BIND=/usr/local/bin/
INCLUDE=-I ../linux/include/uapi

TARGET=femon-s2

all: $(TARGET) $(TARGET)

$(TARGET): $(OBJ)
	$(CC) $(CFLG) $(OBJ) -o $(TARGET) $(CLIB) 

install: all
	cp $(TARGET) $(BIND)

uninstall:
	rm $(BIND)$(TARGET)

clean:
	rm -f $(OBJ) $(TARGET) *~

%.o: %.c
	$(CC) $(INCLUDE) -c $< -o $@
