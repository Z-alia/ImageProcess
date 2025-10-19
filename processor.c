#include "processor.h"
#include <string.h>

// 默认实现：
// 1) 将 original 的 0/255 直接拷贝到 imo，保持黑白一致
// 2) 演示性地对图像四周描一个 1 像素的框线，使用索引颜色 1（红色）
//    仅作为示例，方便用户看到标注区域；如不需要可删除
void process_original_to_imo(const uint8_t **original,
                             uint8_t **imo,
                             int width,
                             int height) {
    if (!original || !imo || width <= 0 || height <= 0) return;

    // 基础：拷贝 0/255 到 imo
    for (int y = 0; y < height; ++y) {
        for (int x = 0; x < width; ++x) {
            uint8_t v = original[y][x];
            // 仅接受 0 或 255，其它值强制归一到最近的黑白
            imo[y][x] = (v < 128) ? 0 : 255;
        }
    }

}
