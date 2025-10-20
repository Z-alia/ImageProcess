#include "processor.h"
#include "global_image_buffer.h"
#include <string.h>
#include <stddef.h>
#ifdef PROCESSOR_VERIFY_BINARY
#include <assert.h>
#endif

#ifndef RESTRICT
#if defined(__GNUC__) || defined(__clang__)
#define RESTRICT __restrict__
#elif defined(_MSC_VER)
#define RESTRICT __restrict
#else
#define RESTRICT
#endif
#endif

// 默认实现：
// 1) 将 original 的 0/255 直接拷贝到 imo，保持黑白一致
// 2) 演示性地对图像四周描一个 1 像素的框线，使用索引颜色 1（红色）
//    仅作为示例，方便用户看到标注区域；如不需要可删除
void process_original_to_imo(const uint8_t * RESTRICT original,
                             uint8_t * RESTRICT imo,
                             int width,
                             int height) {
    if (width <= 0 || height <= 0) return;

#ifdef PROCESSOR_VERIFY_BINARY
    // 可选的调试校验：确保输入严格为 0/255
    for (int i = 0, total = width * height; i < total; ++i) {
        assert(original[i] == 0 || original[i] == 255);
    }
#endif

#ifdef SANITIZE_INPUT
    // 若需要容错净化，就地阈值归一化到 0/255（分支消除写法）
    for (int i = 0, total = width * height; i < total; ++i) {
        uint8_t v = original[i];
        imo[i] = (uint8_t)-(v >> 7); // v>=128 -> 0xFF，否则 0x00
    }
#else
    // 默认：契约保证 original 已是 0/255，直接拷贝最快
    memcpy(imo, original, (size_t)width * (size_t)height);
#endif
}
