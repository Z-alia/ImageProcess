#ifndef GLOBAL_IMAGE_BUFFER_H
#define GLOBAL_IMAGE_BUFFER_H
#include <stdint.h>

#define IMAGE_H 120
#define IMAGE_W 188

extern uint8_t original_bi_image[IMAGE_H][IMAGE_W];
extern uint8_t imo[IMAGE_H][IMAGE_W];
extern uint8_t Grayscale[IMAGE_H][IMAGE_W];

#endif // GLOBAL_IMAGE_BUFFER_H
