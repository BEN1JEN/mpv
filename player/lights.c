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

void update_lights(int width, int height, enum mp_imgfmt imgfmt, uint8_t *yPlane, uint8_t *uPlane, uint8_t *vPlane) {
	if (imgfmt != 1002) { // YUV420
		return;
	}

	printf("\x1b[H");
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
	fflush(stdout);
}
