#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <fcntl.h>
#include <string.h>
#include <time.h>
#define FILE_NAME "/dev/msgboard1"

char line[20];

void clear(void);

int send_cmd(int fd, char* cmd, size_t length) {
	int retval;
	retval = write (fd, cmd, length);
	if (retval < 0) {
		fprintf(stderr, "An error occured\n");
	}
	return retval;
}

void draw_pixel(int fd) {
	/*maximum x value is 127 which fits perfectly
	  into a signed char so no need to use ints
	  (max y is much lower due to the shape of the screen) */
	int x = -1;
	while (x < 0) {
		printf ("\nPlease provide a value for 0<=x<128 ");
		fgets(line, sizeof(line), stdin);
		sscanf(line, "%d", &x);
		if(line[strlen(line) - 1] != '\n')
			clear();
	}
	int y = -1;
	while (y < 0) {
		printf ("Please provide a value for 0<=y<128 ");
		fgets(line, sizeof(line), stdin);
		sscanf(line, "%d", &y);
		if(line[strlen(line) - 1] != '\n')
			clear();
	}
	printf ("Sending data...\n");
	char cmd[3];
	cmd[0] = 1;
	cmd[1] = (char) x;
	cmd[2] = (char) y;
	send_cmd(fd, cmd, 3);
}

void draw_line(int fd) {
	int x = -1;
	while (x < 0) {
		printf ("Please provide a value for 0<=x1<128 ");
		fgets(line, sizeof(line), stdin);
		sscanf(line, "%d", &x);
		if(line[strlen(line) - 1] != '\n')
			clear();
	}
	int y = -1;
	while (y < 0) {
		printf ("Please provide a value for 0<=y1<128 ");
		fgets(line, sizeof(line), stdin);
		sscanf(line, "%d", &y);
		if(line[strlen(line) - 1] != '\n')
			clear();
	}
	int w = -1;
	while (w < 0) {
		printf ("Please provide a value for 0<x2<128 ");
		fgets(line, sizeof(line), stdin);
		sscanf(line, "%d", &w);
		if(line[strlen(line) - 1] != '\n')
			clear();
	}
	int h = -1;
	while (h < 0) {
		printf ("Please provide a value for 0<=y2<128 ");
		fgets(line, sizeof(line), stdin);
		sscanf(line, "%d", &h);
		if(line[strlen(line) - 1] != '\n')
			clear();
	}
	printf ("Sending data...\n");
	char cmd[5];
	cmd[0] = 2;
	cmd[1] = (char) x;
	cmd[2] = (char) y;
	cmd[3] = (char) w;
	cmd[4] = (char) h;
	send_cmd(fd, cmd, 5);
}

void draw_square(int fd) {
	int x = -1;
	while (x < 0) {
		printf ("Please provide a value for 0<=x1<128 ");
		fgets(line, sizeof(line), stdin);
		sscanf(line, "%d", &x);
		if(line[strlen(line) - 1] != '\n')
			clear();
	}
	int y = -1;
	while (y < 0) {
		printf ("Please provide a value for 0<=y1<128 ");
		fgets(line, sizeof(line), stdin);
		sscanf(line, "%d", &y);
		if(line[strlen(line) - 1] != '\n')
			clear();
	}
	int w = -1;
	while (w < 0) {
		printf ("Please provide a value for 0<x2<128 ");
		fgets(line, sizeof(line), stdin);
		sscanf(line, "%d", &w);
		if(line[strlen(line) - 1] != '\n')
			clear();
	}
	int h = -1;
	while (h < 0) {
		printf ("Please provide a value for 0<=y2<128 ");
		fgets(line, sizeof(line), stdin);
		sscanf(line, "%d", &h);
		if(line[strlen(line) - 1] != '\n')
			clear();
	}
	printf ("Sending data...\n");
	char cmd[5];
	cmd[0] = 3;
	cmd[1] = (char) x;
	cmd[2] = (char) y;
	cmd[3] = (char) w;
	cmd[4] = (char) h;
	send_cmd(fd, cmd, 5);
}

void print_date(int fd) {
	time_t t = time(NULL);
	struct tm tm = *localtime(&t);
	printf("now: %d-%d-%d %d:%d:%d\n", tm.tm_year + 1900, tm.tm_mon + 1, tm.tm_mday, tm.tm_hour, tm.tm_min, tm.tm_sec);
	line[0] = 4;
	line[1] = tm.tm_mday / 10 + '0';
	line[2] = tm.tm_mday % 10 + '0';
	line[3] = '/';
	line[4] = (tm.tm_mon + 1) / 10 + '0';
	line[5] = (tm.tm_mon + 1) % 10 + '0';
	line[6] = '/';
	line[7] = (tm.tm_year + 1900) / 1000 + '0';
	line[8] = ((tm.tm_year + 1900) / 100) % 10 + '0';
	line[9] = ((tm.tm_year + 1900) / 10) % 10 + '0';
	line[10] = (tm.tm_year + 1900) % 10 + '0';
	line[11] = ' ';
	line[12] = tm.tm_hour / 10 + '0';
	line[13] = tm.tm_hour % 10 + '0';
	line[14] = ':';
	line[15] = tm.tm_min / 10 + '0';
	line[16] = tm.tm_min % 10 + '0';
	line[17] = ':';
	line[18] = tm.tm_sec / 10 + '0';
	line[19] = tm.tm_sec % 10 + '0';
	printf ("sending data...\n");
	send_cmd(fd, line, 20);
}
void buzz(int fd) {
	int times = - 1;
	while (times < 0 || times > 127) {
		printf ("How many buzzes? (max 127)\n");
		fgets(line, sizeof(line), stdin);
		sscanf(line, "%d", &times);
		if(line[strlen(line) - 1] != '\n')
			clear();
	}
	char cmd[2];
	cmd[0] = 5;
	cmd[1] = (char) times;
	printf ("sending data...\n");
	send_cmd(fd, cmd, 2);
}
void invert_screen(int fd, int is_inverted) {
	char cmd[2];
	cmd[0] = 6;
	cmd[1] = is_inverted;
	send_cmd(fd, cmd, 2);
}

int main(void) {;
	int choice = 0;
	int is_inverted = 0;
	int fd = open(FILE_NAME, O_RDWR);
	if (fd == -1) {
		fprintf(stderr, "Opening failed\n");
		return EXIT_FAILURE;
	}
	printf ("Connecting to device...\n");
	/* connect to the device */
	char c = 0;
	if (send_cmd(fd, &c, 1) < 0) {
		printf ("Failed to connect\n");
		return EXIT_FAILURE;
	}
	printf ("Connected successfully\n");
	do {
		printf ("\n1. Draw pixel\n");
		printf ("2. Draw line\n");
		printf ("3. Draw square\n");
		printf ("4. Print date\n");
		printf ("5. Buzz\n");
		printf ("6. Invert the screen\n");
		printf ("7. Clear the screen\n");
		printf ("8. Exit\n");
		printf("Type your choice: ");
		fgets(line, sizeof(line), stdin);
		sscanf(line, "%d", &choice);
		if(line[strlen(line) - 1] != '\n')
			clear();
		switch (choice) {
			case 1: draw_pixel(fd);
				break;
			case 2: draw_line(fd);
				break;
			case 3: draw_square(fd);
				break;
			case 4: print_date(fd);
				break;
			case 5: buzz(fd);
				break;
			case 6: if (!is_inverted) {
					invert_screen(fd, 1);
					is_inverted = 1;
				}
				else {
					invert_screen(fd, 0);
					is_inverted = 0;
				}
				break;
			case 7: send_cmd(fd, (char*) &choice, 1);
				break;
		}
	} while(choice != 8);
	printf ("Disconnecting...\n");
	if (close(fd) < 0)
		printf ("Error disconnecting\n");
    return 0;
}

void clear(void) {
	int ch;
	while ((ch = getchar() != '\n') && (ch != EOF));
}

