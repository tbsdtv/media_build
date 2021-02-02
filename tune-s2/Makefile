SRC=kb.c diseqc.c tune-s2.c
HED=kb.h diseqc.h tune-s2.h
OBJ=kb.o diseqc.o tune-s2.o

BIND=/usr/local/bin/

TARGET=tune-s2

all: $(TARGET)

$(TARGET): $(OBJ)
	$(CC) $(CFLG) $(OBJ) -o $(TARGET) $(CLIB) -lm

$(OBJ): $(HED)

install: all
	cp $(TARGET) $(BIND)

uninstall:
	rm $(BIND)$(TARGET)

clean:
	rm -f $(OBJ) $(TARGET) *~ ._*

%.o: %.c
	$(CC) $(INCLUDE) -c $< -o $@
