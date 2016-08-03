#include <linux/types.h>
#include <linux/rtc.h>

/*lengths of the different messages*/
#define DRAW_PIXEL_LGTH 8
#define DRAW_LINE_LGTH 10
#define DRAW_SQUARE_LGTH 10
#define CONNECT_LENGTH 7
#define CHNG_LIGHT_LGTH 7
#define BUZZ_LGTH 7
#define INVERT_LGTH 7
#define CLR_SCREEN_LGTH 6

u8 calculate_checksum(u8* bytes, int length) {
	int i, sum;
	u8 result;
	sum = 0;
	/*only look at those bytes after
	  starting 0xAA and  before the last two*/
	for(i = 1; i < length - 2; i++) {
		sum += bytes[i];
	}
	sum %= 256;
	result = (u8) sum;
	return result;
}

int draw_pixel(u8* out, u8 x, u8 y) {
	out[0] = 0xAA;
	out[1] = 0x08;
	out[2] = 0x0;
	out[3] = 0x09;
	out[4] = x;
	out[5] = y;
	out[6] = calculate_checksum(out, DRAW_PIXEL_LGTH);
	out[7] = 0x55;
	return DRAW_PIXEL_LGTH;
}

int connect(u8* out) {
	out[0] = 0xAA;
	out[1] = 0x07;
	out[2] = 0x0;
	out[3] = 0x06;
	out[4] = 0x01;
	out[5] = 0x0e;
	out[6] = 0x55;
	return CONNECT_LENGTH;
}

int draw_line(u8* out, u8 x, u8 y, u8 l, u8 w) {
	out[0] = 0xAA;
	out[1] = 0x10;
	out[2] = 0x0;
	out[3] = 0x12;
	out[4] = x;
	out[5] = y;
	out[6] = l;
	out[7] = w;
	out[8] = calculate_checksum(out, DRAW_LINE_LGTH);
	out[9] = 0x55;
	return DRAW_SQUARE_LGTH;
}

int draw_square(u8* out, u8 x, u8 y, u8 l, u8 w) {
	out[0] = 0xAA;
	out[1] = 0x10;
	out[2] = 0x0;
	out[3] = 0x07;
	out[4] = x;
	out[5] = y;
	out[6] = l;
	out[7] = w;
	out[8] = calculate_checksum(out, DRAW_SQUARE_LGTH);
	out[9] = 0x55;
	return DRAW_SQUARE_LGTH;
}

int write_date(u8* out, u8* text, u8 length) {
	u8 i = 0;
	out[0] = 0xAA;
	out[1] = 0x1C;
	out[2] = 0x0;
	out[3] = 0x04;
	out[4] = 0x0;
	out[5] = 0x0;
	out[6] = 0x80;
	for(;i < length; i++) {
		out[7 + i] = text[i];
	}
	out[7 + i] = 0x0;
	out[8 + i] = calculate_checksum(out, 10 + i);
	out[9 + i] = 0x55;
	out[10 + i] = 0x0;
	out[11 + i] = 0x0;
	out[12 + i] = 0x0;
	out[13 + i] = 0x0;
	out[14 + i] = 0x0;
	out[15 + i] = 0x0;
	out[16 + i] = 0x0;
	out[17 + i] = 0x0;
	return 17 + i;
}

int buzz(u8* out, u8 times) {
	out[0] = 0xAA;
	out[1] = 0x07;
	out[2] = 0x0;
	out[3] = 0x06;
	out[4] = times;
	out[5] = calculate_checksum(out, BUZZ_LGTH);
	out[6] = 0x55;
	return BUZZ_LGTH;
}

int invert_screen(u8* out, u8 is_inverted) {
	out[0] = 0xAA;
	out[1] = 0x07;
	out[2] = 0x0;
	out[3] = 0x15;
	out[4] = is_inverted ? 0x0 : 0x1;
	out[5] = calculate_checksum(out, INVERT_LGTH);
	out[6] = 0x55;
	return INVERT_LGTH;
}
int clear_screen(u8* out) {
	out[0] = 0xAA;
	out[1] = 0x06;
	out[2] = 0x0;
	out[3] = 0x02;
	out[4] = 0x08;
	out[5] = 0x55;
	return CLR_SCREEN_LGTH;
}

