#include <opencv2/opencv.hpp>
#include <iostream>
#include <filesystem>
#include <vector>
#include <string>
#include <cstdint>
#include "processor.h"
#include "global_image_buffer.h"

namespace fs = std::filesystem;

// 小契约：
// 输入：mp4 文件路径，输出目录，可选是否仅导出 PNG 或同时调用原有处理逻辑
// 输出：将每一帧写出为 PNG（frame_000001.png 等）；另外按 188x120 的尺寸二值化到 original 并调用 process_original_to_imo 生成 imo，可选落盘
// 异常：当视频无法打开、写盘失败、OpenCV 不存在时退出非 0

static void ensure_dir(const fs::path &p) {
    std::error_code ec;
    if (!fs::exists(p, ec)) {
        fs::create_directories(p, ec);
        if (ec) {
            throw std::runtime_error("无法创建输出目录: " + p.string());
        }
    }
}

static void resize_and_binarize(const cv::Mat &src, std::vector<std::vector<uint8_t>> &original,
                                int target_w, int target_h) {
    cv::Mat gray = (src.channels() == 1) ? src : cv::Mat();
    if (gray.empty()) {
        cv::cvtColor(src, gray, cv::COLOR_BGR2GRAY);
    }
    cv::Mat resized;
    cv::resize(gray, resized, cv::Size(target_w, target_h), 0, 0, cv::INTER_LINEAR);

    original.assign(target_h, std::vector<uint8_t>(target_w, 255));
    for (int y = 0; y < target_h; ++y) {
        const uint8_t *row = resized.ptr<uint8_t>(y);
        for (int x = 0; x < target_w; ++x) {
            original[y][x] = (row[x] > 128 ? 255 : 0);
        }
    }
}

static void save_png(const fs::path &outPath, const cv::Mat &img) {
    std::vector<int> params = {cv::IMWRITE_PNG_COMPRESSION, 3};
    if (!cv::imwrite(outPath.string(), img, params)) {
        throw std::runtime_error("写入 PNG 失败: " + outPath.string());
    }
}

int main(int argc, char **argv) {
    if (argc < 3) {
        std::cerr << "用法: video_processor <input.mp4> <output_dir> [--export-imo]" << std::endl;
        std::cerr << "  input.mp4   - 输入视频文件路径" << std::endl;
        std::cerr << "  output_dir  - 输出目录路径" << std::endl;
        std::cerr << "  --export-imo - (可选) 同时导出处理后的imo图像" << std::endl;
        return 2;
    }
    
    const fs::path input = argv[1];
    const fs::path outDir = argv[2];
    const bool exportImo = (argc >= 4 && std::string(argv[3]) == "--export-imo");
    
    // 验证输入文件
    if (!fs::exists(input)) {
        std::cerr << "错误: 输入文件不存在: " << input << std::endl;
        return 1;
    }

    try {
        ensure_dir(outDir);
    } catch (const std::exception &e) {
        std::cerr << "错误: " << e.what() << std::endl;
        return 3;
    }

    cv::VideoCapture cap(input.string());
    if (!cap.isOpened()) {
        std::cerr << "错误: 无法打开视频: " << input << std::endl;
        return 4;
    }
    
    // 获取视频信息
    int total_frames = static_cast<int>(cap.get(cv::CAP_PROP_FRAME_COUNT));
    double fps = cap.get(cv::CAP_PROP_FPS);
    int width = static_cast<int>(cap.get(cv::CAP_PROP_FRAME_WIDTH));
    int height = static_cast<int>(cap.get(cv::CAP_PROP_FRAME_HEIGHT));
    
    std::cout << "视频信息:" << std::endl;
    std::cout << "  分辨率: " << width << "x" << height << std::endl;
    std::cout << "  帧率: " << fps << " fps" << std::endl;
    std::cout << "  总帧数: " << total_frames << std::endl;
    std::cout << "  输出目录: " << outDir << std::endl;
    if (exportImo) {
        std::cout << "  处理模式: 导出原始帧 + imo处理结果" << std::endl;
    } else {
        std::cout << "  处理模式: 仅导出原始帧" << std::endl;
    }
    std::cout << std::endl;

    const int TARGET_W = 188;
    const int TARGET_H = 120;

    cv::Mat frame;
    int idx = 0;
    int progress_interval = std::max(1, total_frames / 20); // 每5%显示一次进度
    
    std::cout << "开始处理..." << std::endl;
    
    while (true) {
        if (!cap.read(frame)) break;
        ++idx;
        
        // 显示进度
        if (idx % progress_interval == 0 || idx == total_frames) {
            int progress = (idx * 100) / std::max(1, total_frames);
            std::cout << "\r进度: " << progress << "% (" << idx << "/" << total_frames << ")" << std::flush;
        }

        // 导出原始帧 PNG（按原分辨率）
        char namebuf[64];
        std::snprintf(namebuf, sizeof(namebuf), "frame_%06d.png", idx);
        fs::path outPng = outDir / namebuf;
        try {
            save_png(outPng, frame);
        } catch (const std::exception &e) {
            std::cerr << "\n错误: " << e.what() << std::endl;
            return 5;
        }

        // 转换成 188x120 二值 original，并调用现有 C 处理逻辑，选择性落盘
        if (exportImo) {
            std::vector<std::vector<uint8_t>> original;
            resize_and_binarize(frame, original, TARGET_W, TARGET_H);
            
            // 拷贝到全局 original_bi_image
            for (int y = 0; y < TARGET_H; ++y) {
                memcpy(original_bi_image[y], original[y].data(), TARGET_W);
            }
            
            // 清空全局 imo
            for (int y = 0; y < TARGET_H; ++y) {
                for (int x = 0; x < TARGET_W; ++x) {
                    imo[y][x] = 255;
                }
            }
            
            process_original_to_imo(&original_bi_image[0][0], &imo[0][0], TARGET_W, TARGET_H);
            process_original_to_imo(&original_bi_image[0][0], &imo[0][0], TARGET_W, TARGET_H);
            
            // 将 imo 可视化落盘为彩色 PNG（0=黑，1=红，2=橙，3=黄，4=绿，5=青，255=白）
            cv::Mat viz(TARGET_H, TARGET_W, CV_8UC3);
            // 颜色映射表
            static const cv::Vec3b colorMap[] = {
                {0,0,0}, {0,0,255}, {0,165,255}, {0,255,255}, 
                {0,255,0}, {255,255,0}, {255,255,255}  // 索引0-5+默认
            };
            
            for (int y = 0; y < TARGET_H; ++y) {
                cv::Vec3b *row = viz.ptr<cv::Vec3b>(y);
                for (int x = 0; x < TARGET_W; ++x) {
                    uint8_t v = imo[y][x];
                    row[x] = (v <= 5) ? colorMap[v] : colorMap[6];  // 255或其他→白色
                }
            }
            std::snprintf(namebuf, sizeof(namebuf), "imo_%06d.png", idx);
            fs::path outImo = outDir / namebuf;
            try {
                save_png(outImo, viz);
            } catch (const std::exception &e) {
                std::cerr << "\n错误: " << e.what() << std::endl;
                return 6;
            }
        }
    }
    
    std::cout << "\n完成！" << std::endl;
    std::cout << "导出帧数: " << idx << std::endl;
    std::cout << "输出目录: " << outDir << std::endl;
    return 0;
}
