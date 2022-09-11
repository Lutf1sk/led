#include <curses.h>
#include <locale.h>
#include <stdio.h>

#include <termios.h>

int main() {
	struct termios term;
	tcgetattr(1, &term);
	term.c_lflag &= ~ECHO;
	term.c_lflag &= ~ICANON;
	tcsetattr(1, TCSANOW, &term);

	tcflush(1, TCIOFLUSH);

	printf("\x1B[?1003h");
	fflush(stdout);

	for (;;) {
		char c = getchar();
		if (c == '\n')
			return 0;
		else if (c <= 31 || c >= 127)
			printf("\\x%0.2x", c);
		else if (c == '\b')
			printf("\\b");
		else
			printf("%c", c);
	}
	return 0;
}

