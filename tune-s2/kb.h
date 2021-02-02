#include <stdlib.h>
#include <string.h>
#include <sys/select.h>
#include <termios.h>
#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>

void reset_terminal_mode();
void set_conio_terminal_mode();
int kbhit();
int kbgetchar();
char getch(void);

