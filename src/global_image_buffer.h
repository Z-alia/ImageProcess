#ifndef GLOBAL_IMAGE_BUFFER_H
#define GLOBAL_IMAGE_BUFFER_H
#include <stdint.h>

#define IMAGE_H 120
#define IMAGE_W 188
#ifdef __cplusplus
extern "C" {
#endif

extern uint8_t original_bi_image[IMAGE_H][IMAGE_W];
extern uint8_t imo[IMAGE_H][IMAGE_W];
extern uint8_t Grayscale[IMAGE_H][IMAGE_W];

// 初始化默认内容为白色（255），用于 GUI 初始展示更美观
void init_global_image_buffers_default(void);
#ifdef __cplusplus
}
#endif

#endif // GLOBAL_IMAGE_BUFFER_H
