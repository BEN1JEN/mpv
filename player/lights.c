#include <errno.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <arpa/inet.h>
#include <netinet/in.h>
#include <netdb.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

#include "lights.h"

typedef uint32_t rgbPixel_t;

static rgbPixel_t getPixel(int width, int height, uint8_t *yPlane, uint8_t *uPlane, uint8_t *vPlane, uint32_t xPos, uint32_t yPos) {
	if (xPos >= width || yPos >= height) {
		return 0x808080;
	}

	uint8_t y = yPlane[xPos+yPos*width];
	uint8_t u = uPlane[xPos/2+yPos/4*width];
	uint8_t v = vPlane[xPos/2+yPos/4*width];

	int r, g, b;
	r = y + (1.370705 * (v-128));
	g = y - (0.698001 * (v-128)) - (0.337633 * (u-128));
	b = y + (1.732446 * (u-128));
	if (r > 255) r = 255;
	if (g > 255) g = 255;
	if (b > 255) b = 255;
	if (r < 0) r = 0;
	if (g < 0) g = 0;
	if (b < 0) b = 0;

	return r | (g << 8) | (b << 16);
}

static bool inited = false;
static int socketFile = 0;

static char const initMsg[] = "control,0,100\n\0";
static void init(void) {
	socketFile = socket(AF_INET, SOCK_STREAM, 0);
	if (socketFile < 0) {
		perror("socket");
		return;
	}
	struct sockaddr_in addr;
	addr.sin_family = AF_INET;
	addr.sin_port = htons(10301);
	addr.sin_addr.s_addr = inet_addr("192.168.2.106");
	if (connect(socketFile, (struct sockaddr *)&addr, sizeof addr) < 0) {
		perror("connect");
		return;
	}
	write(socketFile, &initMsg, sizeof initMsg);
	printf("Done Init.\n");
	inited = true;
}

void update_lights(int width, int height, enum mp_imgfmt imgfmt, uint8_t *yPlane, uint8_t *uPlane, uint8_t *vPlane) {
	if (imgfmt != 1002) { // YUV420
		return;
	}

	if (inited) {
		char lightsBuf[4096];
		static const int LIGHT_COUNT = 100, TARGET_ID = 10;
		int pos = snprintf(&lightsBuf[0], sizeof lightsBuf, "%d,%d\n", LIGHT_COUNT, TARGET_ID);
		write(socketFile, &lightsBuf[0], pos);
		pos = 0;
		for (int i = 0; i < LIGHT_COUNT; i++) {
			uint32_t pix = getPixel(width, height, yPlane, uPlane, vPlane, i*width/LIGHT_COUNT, height/2);
			lightsBuf[pos++] = pix&255;
			lightsBuf[pos++] = (pix>>8)&255;
			lightsBuf[pos++] = (pix>>16)&255;
		}
		lightsBuf[pos++] = 0;
		write(socketFile, &lightsBuf, pos);
		read(socketFile, &lightsBuf, 4);
	} else {
		init();
	}

	/*printf("\x1b[H");
	for (int y = 0; y < height-20; y += 40) {
		for (int x = 0; x < width; x += 20) {
			uint32_t top = getPixel(width, height, yPlane, uPlane, vPlane, x, y);
			uint32_t bottom = getPixel(width, height, yPlane, uPlane, vPlane, x, y+20);
			printf(
				"\x1b[38;2;%u;%u;%um\x1b[48;2;%u;%u;%um🬎",
				top & 255, (top >> 8) & 255, (top >> 16) & 255,
				bottom & 255, (bottom >> 8) & 255, (bottom >> 16) & 255
			);
		}
		putchar('\n');
	}
	printf("\x1b[0m\n\n");
	fflush(stdout);*/
}
