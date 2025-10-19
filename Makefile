CC = gcc
CFLAGS = -Wall -g
PKG_CONFIG ?= pkg-config

# 检查是否安装了必要的包
ifeq ($(shell $(PKG_CONFIG) --exists gtk+-3.0 libpng libjpeg && echo yes),yes)
	LIBS = $(shell $(PKG_CONFIG) --cflags --libs gtk+-3.0 libpng libjpeg)
	DEPS_AVAILABLE = yes
else
	DEPS_AVAILABLE = no
endif

TARGET = imageprocessor
SRCS = main.c
OBJS = $(SRCS:.c=.o)

ifeq ($(DEPS_AVAILABLE),yes)
all: $(TARGET)
else
all:
	@echo "警告: 未找到必要的开发库"
	@echo "请运行 'make install-deps' 或手动安装:"
	@echo "  Ubuntu/Debian: sudo apt-get install libgtk-3-dev libpng-dev libjpeg-dev"
	@echo "  CentOS/RHEL:   sudo yum install gtk3-devel libpng-devel libjpeg-devel"
	@echo "  Fedora:        sudo dnf install gtk3-devel libpng-devel libjpeg-devel"
	@exit 1
endif

$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) -o $(TARGET) $(OBJS) $(LIBS)

%.o: %.c
	$(CC) $(CFLAGS) $(LIBS) -c $< -o $@

clean:
	rm -f $(OBJS) $(TARGET)

install-deps:
ifeq ($(shell grep -q Ubuntu /etc/os-release 2>/dev/null && echo yes),yes)
	sudo apt-get update
	sudo apt-get install libgtk-3-dev libpng-dev libjpeg-dev build-essential
else ifeq ($(shell grep -q debian /etc/os-release 2>/dev/null && echo yes),yes)
	sudo apt-get update
	sudo apt-get install libgtk-3-dev libpng-dev libjpeg-dev build-essential
else ifeq ($(shell grep -q centos /etc/os-release 2>/dev/null && echo yes),yes)
	sudo yum install gtk3-devel libpng-devel libjpeg-devel
else ifeq ($(shell grep -q fedora /etc/os-release 2>/dev/null && echo yes),yes)
	sudo dnf install gtk3-devel libpng-devel libjpeg-devel
else
	@echo "未知的Linux发行版，请手动安装GTK3、libpng和libjpeg开发库"
	@exit 1
endif

check-deps:
	@echo "检查依赖..."
ifeq ($(DEPS_AVAILABLE),yes)
	@echo "✓ 找到所有必要依赖"
	@echo "GTK+ 3.0、libpng 和 libjpeg 可用"
else
	@echo "✗ 缺少必要依赖"
	@echo "请运行 'make install-deps' 安装所需库"
	@exit 1
endif

.PHONY: all clean install-deps check-deps