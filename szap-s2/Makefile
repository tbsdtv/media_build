CC ?= gcc

SRC=lnb.c szap-s2.c tzap-t2.c
HED=lnb.h
OBJ1=lnb.o szap-s2.o
OBJ2=tzap-t2.o lnb.o

BIND=/usr/local/bin/
INCLUDE=-I ../linux/include/uapi

TARGET1=szap-s2
TARGET2=tzap-t2

all: $(TARGET1) $(TARGET2)

$(TARGET1): $(OBJ1)
	$(CC) $(CFLG) $(OBJ1) -o $(TARGET1) $(CLIB) 

$(TARGET2): $(OBJ2)
	$(CC) $(CFLG) $(OBJ2) -o $(TARGET2) $(CLIB) 

$(OBJ): $(HED)

install: all
	cp $(TARGET1) $(BIND)
	cp $(TARGET2) $(BIND)

uninstall:
	rm $(BIND)$(TARGET1)
	rm $(BIND)$(TARGET2)

clean:
	rm -f $(OBJ1) $(TARGET1) *~
	rm -f $(OBJ2) $(TARGET2) *~

%.o: %.c
	$(CC) $(INCLUDE) -c $< -o $@
