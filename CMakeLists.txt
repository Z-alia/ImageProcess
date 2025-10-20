cmake_minimum_required(VERSION 3.16)
project(ImageProcessor C)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS OFF)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin")

set(SOURCES
    main.c
    processor.c
)

find_package(PkgConfig REQUIRED)
pkg_check_modules(GTK3 REQUIRED IMPORTED_TARGET gtk+-3.0)
pkg_check_modules(LIBPNG REQUIRED IMPORTED_TARGET libpng)
pkg_check_modules(LIBJPEG REQUIRED IMPORTED_TARGET libjpeg)

add_executable(imageprocessor ${SOURCES})

target_link_libraries(imageprocessor
    PRIVATE
        PkgConfig::GTK3
        PkgConfig::LIBPNG
        PkgConfig::LIBJPEG
)
