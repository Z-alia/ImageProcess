#include "global_image_buffer.h"

uint8_t original_bi_image[IMAGE_H][IMAGE_W] = {0};
uint8_t imo[IMAGE_H][IMAGE_W] = {0};
uint8_t Grayscale[IMAGE_H][IMAGE_W] = {0};

void init_global_image_buffers_default(void)
{
	for (int y = 0; y < IMAGE_H; ++y) {
		for (int x = 0; x < IMAGE_W; ++x) {
			original_bi_image[y][x] = 255;
			imo[y][x] = 255;
			Grayscale[y][x] = 255;
		}
	}
}
