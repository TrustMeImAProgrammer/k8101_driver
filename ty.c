#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
struct command {
	int type;
	char* text;
};

void clear(void) {
	int ch;
	while ((ch = getchar() != '\n') && (ch != EOF));
}


char b[100];
int i;
int main(int argc, char *argv[])
{
	time_t t = time(NULL);
	struct tm tm = *localtime(&t);
	printf("now: %d-%d-%d %d:%d:%d\n", tm.tm_year + 1900, tm.tm_mon + 1, tm.tm_mday, tm.tm_hour, tm.tm_min, tm.tm_sec);
	printf("%c", 2 + '0');
    return 0;
}
