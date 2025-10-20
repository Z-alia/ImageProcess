#include <opencv2/opencv.hpp>
#include <iostream>
#include <filesystem>
#include <vector>
#include <string>
#include <cstdint>
#include "processor.h"

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
    cv::Mat gray, resized;
    if (src.channels() == 1) {
        gray = src;
    } else {
        cv::cvtColor(src, gray, cv::COLOR_BGR2GRAY);
    }
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
        return 2;
    }
    const fs::path input = argv[1];
    const fs::path outDir = argv[2];
    const bool exportImo = (argc >= 4 && std::string(argv[3]) == "--export-imo");

    try {
        ensure_dir(outDir);
    } catch (const std::exception &e) {
        std::cerr << e.what() << std::endl;
        return 3;
    }

    cv::VideoCapture cap(input.string());
    if (!cap.isOpened()) {
        std::cerr << "无法打开视频: " << input << std::endl;
        return 4;
    }

    const int TARGET_W = 188;
    const int TARGET_H = 120;

    cv::Mat frame;
    int idx = 0;
    while (true) {
        if (!cap.read(frame)) break;
        ++idx;

        // 导出原始帧 PNG（按原分辨率）
        char namebuf[64];
        std::snprintf(namebuf, sizeof(namebuf), "frame_%06d.png", idx);
        fs::path outPng = outDir / namebuf;
        try {
            save_png(outPng, frame);
        } catch (const std::exception &e) {
            std::cerr << e.what() << std::endl;
            return 5;
        }

        // 转换成 188x120 二值 original，并调用现有 C 处理逻辑，选择性落盘
        std::vector<std::vector<uint8_t>> original;
        resize_and_binarize(frame, original, TARGET_W, TARGET_H);

        // 为 C 接口准备指针数组
        std::vector<uint8_t*> imoRows(TARGET_H, nullptr);
        std::vector<const uint8_t*> origRows(TARGET_H, nullptr);
        std::vector<std::vector<uint8_t>> imo(TARGET_H, std::vector<uint8_t>(TARGET_W, 255));
        for (int y = 0; y < TARGET_H; ++y) {
            origRows[y] = original[y].data();
            imoRows[y] = imo[y].data();
        }
        process_original_to_imo(origRows.data(), imoRows.data(), TARGET_W, TARGET_H);

        if (exportImo) {
            // 将 imo 可视化落盘为彩色 PNG（0=黑，1=红，2=橙，3=黄，4=绿，5=青，255=白）
            cv::Mat viz(TARGET_H, TARGET_W, CV_8UC3);
            for (int y = 0; y < TARGET_H; ++y) {
                cv::Vec3b *row = viz.ptr<cv::Vec3b>(y);
                for (int x = 0; x < TARGET_W; ++x) {
                    uint8_t v = imo[y][x];
                    cv::Vec3b bgr;
                    switch (v) {
                        case 0: bgr = {0,0,0}; break;            // 黑
                        case 1: bgr = {0,0,255}; break;          // 红 (BGR)
                        case 2: bgr = {0,165,255}; break;        // 橙
                        case 3: bgr = {0,255,255}; break;        // 黄
                        case 4: bgr = {0,255,0}; break;          // 绿
                        case 5: bgr = {255,255,0}; break;        // 青
                        case 255: default: bgr = {255,255,255}; break; // 白
                    }
                    row[x] = bgr;
                }
            }
            std::snprintf(namebuf, sizeof(namebuf), "imo_%06d.png", idx);
            fs::path outImo = outDir / namebuf;
            try {
                save_png(outImo, viz);
            } catch (const std::exception &e) {
                std::cerr << e.what() << std::endl;
                return 6;
            }
        }
    }

    std::cout << "完成，导出帧数: " << idx << std::endl;
    return 0;
}
