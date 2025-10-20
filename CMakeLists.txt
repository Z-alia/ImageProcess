cmake_minimum_required(VERSION 3.16)
project(ImageProcessor C CXX)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS OFF)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin")

# 选项：是否构建 GTK 图形界面（默认 OFF，便于在没有 GTK 的环境下也能构建视频处理工具）
option(BUILD_GUI "Build GTK GUI imageprocessor" OFF)

# ---------------- GUI 目标（可选） ----------------
if(BUILD_GUI)
    set(SOURCES_GUI
        main.cpp
        processor.c
    )

    find_package(PkgConfig REQUIRED)
    pkg_check_modules(GTK3 REQUIRED IMPORTED_TARGET gtk+-3.0)
    pkg_check_modules(LIBPNG REQUIRED IMPORTED_TARGET libpng)
    pkg_check_modules(LIBJPEG REQUIRED IMPORTED_TARGET libjpeg)

    # GUI 可选集成 OpenCV（用于视频导入）。未找到 OpenCV 也可以继续构建 GUI，只是禁用视频按钮。
    find_package(OpenCV QUIET)

    add_executable(imageprocessor ${SOURCES_GUI})

    target_link_libraries(imageprocessor
        PRIVATE
            PkgConfig::GTK3
            PkgConfig::LIBPNG
            PkgConfig::LIBJPEG
    )

    if(OpenCV_FOUND)
        target_include_directories(imageprocessor PRIVATE ${OpenCV_INCLUDE_DIRS})
        target_link_libraries(imageprocessor PRIVATE ${OpenCV_LIBS})
        target_compile_definitions(imageprocessor PRIVATE HAVE_OPENCV=1)
    endif()
endif()

# ---------------- 视频逐帧处理 CLI 目标 ----------------
# 依赖 OpenCV：videoio、imgproc、imgcodecs 等模块
option(BUILD_VIDEO_TOOL "Build video_processor CLI (OpenCV)" ON)
if(BUILD_VIDEO_TOOL)
    find_package(OpenCV QUIET)
    if(OpenCV_FOUND)
        add_executable(video_processor
            video_processor.cpp
            processor.c
        )
        target_include_directories(video_processor PRIVATE ${OpenCV_INCLUDE_DIRS})
        target_link_libraries(video_processor PRIVATE ${OpenCV_LIBS})
    else()
        message(WARNING "OpenCV 未找到，将跳过 video_processor 目标的构建。设置 OpenCV 环境或使用 -DOpenCV_DIR 指定后重试。")
    endif()
endif()
