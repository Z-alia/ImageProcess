#include <gtk/gtk.h>
#include <png.h>
#include <jpeglib.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include "processor.h" // 用户自定义 original->imo 处理函数声明

// 全局变量
GtkWidget *window;
GtkWidget *image_area;            // 原始输入图显示（原尺寸）
GtkWidget *original_array_area;   // Original 二值数组图显示
GtkWidget *upload_button;
GtkWidget *process_button;        // 处理按钮
GtkWidget *imo_image_view = NULL; // 右侧 IMO 图
GdkPixbuf *current_image = NULL;  // 持有缩放到目标尺寸的图像

// 用于存储二值图数据的二维数组 (值域 0 或 255)
uint8_t **original_bi_image = NULL;
uint8_t **imo = NULL;             // 最终输出数组（0/255，1/2/3…为颜色索引）
int image_width = 0;
int image_height = 0;

// 目标尺寸 (120行 x 188列)
const int TARGET_WIDTH = 188;
const int TARGET_HEIGHT = 120;

// 函数声明
static void activate(GtkApplication *app, gpointer user_data);
static void upload_image_clicked(GtkWidget *widget, gpointer data);
static void process_image_clicked(GtkWidget *widget, gpointer data);
static gboolean load_png_image(const gchar *filename);
static gboolean load_jpeg_image(const gchar *filename);
static gboolean is_jpeg_file(const gchar *filename);
static gboolean is_png_file(const gchar *filename);
static void free_binary_image_data();
static void reset_image_views();
static GdkPixbuf* scale_pixbuf_to_target(GdkPixbuf *pixbuf);
static void allocate_imo_array();

// 渲染/刷新
static GdkPixbuf* render_bw_array_pixbuf(uint8_t **arr, int w, int h, int pixel_size);
static GdkPixbuf* render_imo_array_pixbuf(uint8_t **arr, int w, int h, int pixel_size);
static void refresh_all_views();
static void on_size_allocate(GtkWidget *widget, GdkRectangle *allocation, gpointer user_data);

int main(int argc, char **argv) {
    GtkApplication *app;
    int status;

    app = gtk_application_new("com.example.binaryimage", G_APPLICATION_FLAGS_NONE);
    g_signal_connect(app, "activate", G_CALLBACK(activate), NULL);
    status = g_application_run(G_APPLICATION(app), argc, argv);
    g_object_unref(app);

    // 释放分配的内存
    free_binary_image_data();

    return status;
}

// 激活应用程序
static void activate(GtkApplication *app, gpointer user_data) {
    // 创建主窗口
    window = gtk_application_window_new(app);
    gtk_window_set_title(GTK_WINDOW(window), "二值图处理器");
    gtk_window_set_default_size(GTK_WINDOW(window), 1000, 700);
    gtk_window_set_position(GTK_WINDOW(window), GTK_WIN_POS_CENTER);

    // 创建垂直布局容器
    GtkWidget *vbox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 10);
    gtk_container_set_border_width(GTK_CONTAINER(vbox), 10);
    gtk_container_add(GTK_CONTAINER(window), vbox);

    // 创建水平按钮布局
    GtkWidget *hbox = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 10);
    gtk_box_pack_start(GTK_BOX(vbox), hbox, FALSE, FALSE, 0);

    // 创建上传按钮
    upload_button = gtk_button_new_with_label("上传PNG/JPEG二值图");
    g_signal_connect(upload_button, "clicked", G_CALLBACK(upload_image_clicked), NULL);
    gtk_box_pack_start(GTK_BOX(hbox), upload_button, FALSE, FALSE, 0);

    // 创建处理按钮
    process_button = gtk_button_new_with_label("处理图像 (original->imo)");
    g_signal_connect(process_button, "clicked", G_CALLBACK(process_image_clicked), NULL);
    gtk_box_pack_start(GTK_BOX(hbox), process_button, FALSE, FALSE, 0);

    // 默认禁用处理按钮
    gtk_widget_set_sensitive(process_button, FALSE);

    // 创建水平布局用于显示图像（左右）
    GtkWidget *image_hbox = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 10);
    gtk_box_pack_start(GTK_BOX(vbox), image_hbox, TRUE, TRUE, 0);

    // 左侧：上下两块（原始输入图 + original 数组图）
    GtkWidget *left_vbox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 10);
    gtk_box_pack_start(GTK_BOX(image_hbox), left_vbox, TRUE, TRUE, 0);

    // 原始输入图框
    GtkWidget *current_frame = gtk_frame_new("原始输入图");
    gtk_frame_set_shadow_type(GTK_FRAME(current_frame), GTK_SHADOW_ETCHED_IN);
    image_area = gtk_image_new();
    gtk_container_add(GTK_CONTAINER(current_frame), image_area);
    gtk_box_pack_start(GTK_BOX(left_vbox), current_frame, TRUE, TRUE, 0);

    // original 数组图框
    GtkWidget *orig_frame = gtk_frame_new("Original 数组图 (0/255)");
    gtk_frame_set_shadow_type(GTK_FRAME(orig_frame), GTK_SHADOW_ETCHED_IN);
    original_array_area = gtk_image_new();
    gtk_container_add(GTK_CONTAINER(orig_frame), original_array_area);
    gtk_box_pack_start(GTK_BOX(left_vbox), orig_frame, TRUE, TRUE, 0);

    // 右侧：IMO 图框
    GtkWidget *imo_frame = gtk_frame_new("IMO 数组图");
    gtk_frame_set_shadow_type(GTK_FRAME(imo_frame), GTK_SHADOW_ETCHED_IN);
    imo_image_view = gtk_image_new();
    gtk_container_add(GTK_CONTAINER(imo_frame), imo_image_view);
    gtk_box_pack_start(GTK_BOX(image_hbox), imo_frame, TRUE, TRUE, 0);

    // 窗口尺寸变化时自适应刷新
    g_signal_connect(current_frame, "size-allocate", G_CALLBACK(on_size_allocate), NULL);
    g_signal_connect(orig_frame, "size-allocate", G_CALLBACK(on_size_allocate), NULL);
    g_signal_connect(imo_frame, "size-allocate", G_CALLBACK(on_size_allocate), NULL);

    // 初始化占位图，避免显示残留
    reset_image_views();

    gtk_widget_show_all(window);
}

// 上传图片按钮点击事件处理函数
static void upload_image_clicked(GtkWidget *widget, gpointer data) {
    GtkWidget *dialog;
    GtkFileChooserAction action = GTK_FILE_CHOOSER_ACTION_OPEN;
    gint res;

    dialog = gtk_file_chooser_dialog_new("选择图像文件",
                                         GTK_WINDOW(window),
                                         action,
                                         "_取消",
                                         GTK_RESPONSE_CANCEL,
                                         "_打开",
                                         GTK_RESPONSE_ACCEPT,
                                         NULL);

    // 设置文件过滤器
    GtkFileFilter *png_filter = gtk_file_filter_new();
    gtk_file_filter_set_name(png_filter, "PNG 文件");
    gtk_file_filter_add_pattern(png_filter, "*.png");
    gtk_file_chooser_add_filter(GTK_FILE_CHOOSER(dialog), png_filter);
    
    GtkFileFilter *jpeg_filter = gtk_file_filter_new();
    gtk_file_filter_set_name(jpeg_filter, "JPEG 文件");
    gtk_file_filter_add_pattern(jpeg_filter, "*.jpg");
    gtk_file_filter_add_pattern(jpeg_filter, "*.jpeg");
    gtk_file_chooser_add_filter(GTK_FILE_CHOOSER(dialog), jpeg_filter);
    
    GtkFileFilter *all_filter = gtk_file_filter_new();
    gtk_file_filter_set_name(all_filter, "所有图像文件");
    gtk_file_filter_add_pattern(all_filter, "*.png");
    gtk_file_filter_add_pattern(all_filter, "*.jpg");
    gtk_file_filter_add_pattern(all_filter, "*.jpeg");
    gtk_file_chooser_add_filter(GTK_FILE_CHOOSER(dialog), all_filter);

    res = gtk_dialog_run(GTK_DIALOG(dialog));
    if (res == GTK_RESPONSE_ACCEPT) {
        char *filename;
        GtkFileChooser *chooser = GTK_FILE_CHOOSER(dialog);
        filename = gtk_file_chooser_get_filename(chooser);
        
        // 加载图像
        gboolean loaded = FALSE;
        if (is_png_file(filename)) {
            g_print("检测到PNG文件: %s\n", filename);
            loaded = load_png_image(filename);
        } else if (is_jpeg_file(filename)) {
            g_print("检测到JPEG文件: %s\n", filename);
            loaded = load_jpeg_image(filename);
        } else {
            g_print("错误: 不支持的图像格式: %s\n", filename);
            GtkWidget *error_dialog = gtk_message_dialog_new(GTK_WINDOW(window),
                                                             GTK_DIALOG_DESTROY_WITH_PARENT,
                                                             GTK_MESSAGE_ERROR,
                                                             GTK_BUTTONS_CLOSE,
                                                             "不支持的图像格式，请选择PNG或JPEG文件");
            gtk_dialog_run(GTK_DIALOG(error_dialog));
            gtk_widget_destroy(error_dialog);
        }
        
        if (loaded) {
            g_print("成功加载图像: %s\n", filename);
            g_print("图像已存储为二维uint8_t数组，尺寸: %d x %d\n", image_width, image_height);
            // 启用上传后的按钮
            gtk_widget_set_sensitive(process_button, TRUE);
            // 清空右侧 IMO 视图
            if (imo_image_view)
                gtk_image_set_from_icon_name(GTK_IMAGE(imo_image_view), "image-x-generic", GTK_ICON_SIZE_DIALOG);
            // 刷新左下 original 数组显示
            refresh_all_views();
        }
        
        g_free(filename);
    }

    gtk_widget_destroy(dialog);
}

// 移除原有弹窗查看逻辑，改为固定面板显示

// 释放imo数组内存
static void free_imo_array() {
    if (imo) {
        for (int i = 0; i < image_height; i++) {
            if (imo[i]) {
                free(imo[i]);
            }
        }
        free(imo);
        imo = NULL;
    }
}

// 释放二值图数据内存
static void free_binary_image_data() {
    if (original_bi_image) {
        for (int i = 0; i < image_height; i++) {
            free(original_bi_image[i]);
        }
        free(original_bi_image);
        original_bi_image = NULL;
    }
    
    // 释放imo数组
    free_imo_array();

    image_width = 0;
    image_height = 0;
}

static void reset_image_views() {
    if (original_array_area) {
        gtk_image_set_from_icon_name(GTK_IMAGE(original_array_area), "image-x-generic", GTK_ICON_SIZE_DIALOG);
    }
    if (imo_image_view) {
        gtk_image_set_from_icon_name(GTK_IMAGE(imo_image_view), "image-x-generic", GTK_ICON_SIZE_DIALOG);
    }
}

// 将Pixbuf缩放到目标尺寸
static GdkPixbuf* scale_pixbuf_to_target(GdkPixbuf *pixbuf) {
    if (!pixbuf) {
        return NULL;
    }
    
    int width = gdk_pixbuf_get_width(pixbuf);
    int height = gdk_pixbuf_get_height(pixbuf);
    
    // 如果已经是目标尺寸，直接返回
    if (width == TARGET_WIDTH && height == TARGET_HEIGHT) {
        return g_object_ref(pixbuf);
    }
    
    // 缩放图像到目标尺寸
    GdkPixbuf *scaled = gdk_pixbuf_scale_simple(
        pixbuf, 
        TARGET_WIDTH, 
        TARGET_HEIGHT, 
        GDK_INTERP_BILINEAR
    );
    
    return scaled;
}

// 检查是否为PNG文件
static gboolean is_png_file(const gchar *filename) {
    FILE *fp = fopen(filename, "rb");
    if (!fp) {
        return FALSE;
    }
    
    png_byte header[8];
    fread(header, 1, 8, fp);
    fclose(fp);
    
    return (png_sig_cmp(header, 0, 8) == 0);
}

// 检查是否为JPEG文件
static gboolean is_jpeg_file(const gchar *filename) {
    FILE *fp = fopen(filename, "rb");
    if (!fp) {
        return FALSE;
    }
    
    unsigned char header[2];
    fread(header, 1, 2, fp);
    fclose(fp);
    
    // JPEG文件以0xFFD8开头
    return (header[0] == 0xFF && header[1] == 0xD8);
}

// 加载PNG图像
static gboolean load_png_image(const gchar *filename) {
    FILE *fp;
    png_byte header[8];
    
    g_print("正在尝试加载PNG文件: %s\n", filename);
    
    // 打开文件
    fp = fopen(filename, "rb");
    if (!fp) {
        g_print("错误: 无法打开文件 '%s'，请检查文件路径和权限\n", filename);
        return FALSE;
    }
    g_print("✓ 文件成功打开\n");
    
    // 检查是否是PNG文件
    fread(header, 1, 8, fp);
    if (png_sig_cmp(header, 0, 8)) {
        g_print("错误: 文件 '%s' 不是有效的PNG文件\n", filename);
        // 检查是否可能是JPEG文件
        if (header[0] == 0xFF && header[1] == 0xD8 && header[2] == 0xFF) {
            g_print("提示: 该文件看起来像是JPEG格式，而非PNG格式\n");
        }
        fclose(fp);
        return FALSE;
    }
    g_print("✓ PNG签名验证通过\n");
    fclose(fp); // 关闭文件，我们不再需要直接读取PNG数据
    
    // 更新图像到界面
    GdkPixbuf *pixbuf = gdk_pixbuf_new_from_file(filename, NULL);
    if (pixbuf) {
        g_print("✓ 成功创建GDK Pixbuf\n");
        
        // 缩放图像到目标尺寸
        GdkPixbuf *scaled_pixbuf = scale_pixbuf_to_target(pixbuf);
        if (!scaled_pixbuf) {
            g_print("错误: 无法缩放图像\n");
            g_object_unref(pixbuf);
            return FALSE;
        }
        g_object_unref(pixbuf); // 释放原始pixbuf
        pixbuf = scaled_pixbuf;
        
        g_print("图像已缩放至: %d x %d\n", TARGET_WIDTH, TARGET_HEIGHT);
        
        // 清除之前的图像
        if (current_image) {
            g_object_unref(current_image);
        }
        
        // 设置新图像（使用原始尺寸图像）
        GdkPixbuf *original_pixbuf = gdk_pixbuf_new_from_file(filename, NULL);
        if (original_pixbuf) {
            gtk_image_set_from_pixbuf(GTK_IMAGE(image_area), original_pixbuf);
            g_object_unref(original_pixbuf);
        } else {
            gtk_image_set_from_pixbuf(GTK_IMAGE(image_area), pixbuf);
        }
        current_image = pixbuf; // 但保持缩放后的图像用于处理
        
        // 清空 左下 original数组 与 右侧 IMO 视图
        if (original_array_area)
            gtk_image_set_from_icon_name(GTK_IMAGE(original_array_area), "image-x-generic", GTK_ICON_SIZE_DIALOG);
        if (imo_image_view)
            gtk_image_set_from_icon_name(GTK_IMAGE(imo_image_view), "image-x-generic", GTK_ICON_SIZE_DIALOG);
        
        // 释放之前的二值图数据
        free_binary_image_data();
        
        // 为后续处理做好准备 - 将图像数据存储为二维uint8_t数组
        image_width = TARGET_WIDTH;
        image_height = TARGET_HEIGHT;
        
        g_print("正在分配内存: %d x %d\n", image_width, image_height);
        
        // 分配二维数组内存
        original_bi_image = (uint8_t **)malloc(image_height * sizeof(uint8_t *));
        if (!original_bi_image) {
            g_print("错误: 无法为图像数据分配内存 (第一维)\n");
            fclose(fp);
            g_object_unref(pixbuf);
            current_image = NULL;
            return FALSE;
        }
        
        for (int i = 0; i < image_height; i++) {
            original_bi_image[i] = (uint8_t *)malloc(image_width * sizeof(uint8_t));
            if (!original_bi_image[i]) {
                g_print("错误: 无法为图像数据分配内存 (第%d行)\n", i);
                // 释放已分配的内存
                for (int j = 0; j < i; j++) {
                    free(original_bi_image[j]);
                }
                free(original_bi_image);
                original_bi_image = NULL;
                
                fclose(fp);
                g_object_unref(pixbuf);
                current_image = NULL;
                return FALSE;
            }
        }
        g_print("✓ 内存分配成功\n");
        
        // 获取缩放后图像的像素数据
        int rowstride = gdk_pixbuf_get_rowstride(pixbuf);
        guchar *pixels = gdk_pixbuf_get_pixels(pixbuf);
        int channels = gdk_pixbuf_get_n_channels(pixbuf);
        
        // 读取图像数据并转换为二值格式
        g_print("正在读取缩放后PNG图像数据...\n");
        int non_binary_pixels = 0;
        for (int y = 0; y < image_height; y++) {
            guchar *row = pixels + y * rowstride;
            for (int x = 0; x < image_width; x++) {
                if (channels == 1) {
                    // 灰度图像
                    int gray = row[x];
                    if (gray != 0 && gray != 255) {
                        non_binary_pixels++;
                    }
                    original_bi_image[y][x] = (gray > 128) ? 255 : 0;
                } else if (channels == 2) {
                    // 灰度+Alpha图像
                    int gray = row[x * 2];
                    int alpha = row[x * 2 + 1];
                    
                    // 如果alpha为0，则为透明（这里当作白色处理）
                    if (alpha == 0) {
                        original_bi_image[y][x] = 255; // 白色
                    } else {
                        if (gray != 0 && gray != 255) {
                            non_binary_pixels++;
                        }
                        original_bi_image[y][x] = (gray > 128) ? 255 : 0;
                    }
                } else if (channels == 3) {
                    // RGB彩色图像
                    int r = row[x * 3];
                    int g = row[x * 3 + 1];
                    int b = row[x * 3 + 2];
                    int gray = (int)(0.299 * r + 0.587 * g + 0.114 * b);
                    
                    if (gray != 0 && gray != 255) {
                        non_binary_pixels++;
                    }
                    original_bi_image[y][x] = (gray > 128) ? 255 : 0;
                } else if (channels == 4) {
                    // RGBA图像
                    int r = row[x * 4];
                    int g = row[x * 4 + 1];
                    int b = row[x * 4 + 2];
                    int a = row[x * 4 + 3];
                    
                    // 如果alpha为0，则为透明（这里当作白色处理）
                    if (a == 0) {
                        original_bi_image[y][x] = 255; // 白色
                    } else {
                        // RGB转灰度
                        int gray = (int)(0.299 * r + 0.587 * g + 0.114 * b);
                        
                        // 检查灰度值是否为二值
                        if (gray != 0 && gray != 255) {
                            non_binary_pixels++;
                        }
                        
                        // 转换为二值：0或255
                        original_bi_image[y][x] = (gray > 128) ? 255 : 0;
                    }
                }
            }
        }
        
        if (non_binary_pixels > 0) {
            g_print("警告: 发现 %d 个非二值像素，已自动转换为二值\n", non_binary_pixels);
        }
        
        g_print("✓ PNG图像数据读取完成\n");
        g_print("✓ 图像数据转换完成\n");
        
        // 为后续处理做好准备
        g_print("图像已加载并存储为二维uint8_t数组\n");
        g_print("图像尺寸: %d x %d\n", image_width, image_height);
    } else {
        g_print("错误: 无法从文件创建GDK Pixbuf\n");
    }
    
    // 不需要清理libpng资源，因为我们没有使用它来解码图像数据
    
    g_print("PNG文件处理完成: %s\n", pixbuf ? "成功" : "失败");
    return pixbuf != NULL;
}

// 添加分配imo数组的函数
static void allocate_imo_array() {
    // 如果imo数组已经存在，先释放它
    free_imo_array();
    
    // 分配imo数组内存
    imo = (uint8_t **)malloc(image_height * sizeof(uint8_t *));
    if (!imo) {
        g_print("错误: 无法为imo数组分配内存 (第一维)\n");
        return;
    }
    
    for (int i = 0; i < image_height; i++) {
        imo[i] = (uint8_t *)malloc(image_width * sizeof(uint8_t));
        if (!imo[i]) {
            g_print("错误: 无法为imo数组分配内存 (第%d行)\n", i);
            // 释放已分配的内存
            for (int j = 0; j < i; j++) {
                free(imo[j]);
            }
            free(imo);
            imo = NULL;
            return;
        }
    }
    
    // 初始化为255(白)以避免未处理时出现随机颜色
    for (int i = 0; i < image_height && imo; i++) {
        if (imo[i]) memset(imo[i], 255, image_width * sizeof(uint8_t));
    }
    g_print("✓ imo数组内存分配成功\n");
}

// 处理图像按钮点击事件处理函数
static void process_image_clicked(GtkWidget *widget, gpointer data) {
    if (!original_bi_image || image_width <= 0 || image_height <= 0) {
        return;
    }
    
    // 分配imo数组内存
    allocate_imo_array();
    
    if (!imo) {
        g_print("错误: 无法分配imo数组内存\n");
        return;
    }
    
    // 调用外部处理函数（由用户在 processor.c 中实现）
    process_original_to_imo((const uint8_t **)original_bi_image, imo, image_width, image_height);
    
    g_print("图像处理完成，imo数组已生成\n");

    // 刷新右侧 IMO 视图
    refresh_all_views();
}
// 移除原有 IMO 弹窗逻辑，改为固定面板显示

// 加载JPEG图像
static gboolean load_jpeg_image(const gchar *filename) {
    FILE *fp;
    struct jpeg_decompress_struct cinfo;
    struct jpeg_error_mgr jerr;
    
    g_print("正在尝试加载JPEG文件: %s\n", filename);
    
    // 打开文件
    fp = fopen(filename, "rb");
    if (!fp) {
        g_print("错误: 无法打开文件 '%s'，请检查文件路径和权限\n", filename);
        return FALSE;
    }
    g_print("✓ 文件成功打开\n");
    
    // 初始化JPEG解压对象
    cinfo.err = jpeg_std_error(&jerr);
    jpeg_create_decompress(&cinfo);
    jpeg_stdio_src(&cinfo, fp);
    
    // 读取JPEG文件头
    jpeg_read_header(&cinfo, TRUE);
    
    g_print("JPEG图像信息: %d x %d, 颜色组件数: %d\n", 
            (int)cinfo.image_width, (int)cinfo.image_height, cinfo.num_components);
    
    // 检查图像尺寸
    if (cinfo.image_width <= 0 || cinfo.image_height <= 0) {
        g_print("错误: 无效的图像尺寸 (%d x %d)\n", (int)cinfo.image_width, (int)cinfo.image_height);
        jpeg_destroy_decompress(&cinfo);
        fclose(fp);
        return FALSE;
    }
    
    // 检查图像尺寸是否过大
    if (cinfo.image_width > 10000 || cinfo.image_height > 10000) {
        g_print("警告: 图像尺寸过大 (%d x %d)，可能导致内存分配问题\n", 
                (int)cinfo.image_width, (int)cinfo.image_height);
    }
    
    // 我们不需要使用libjpeg来读取像素数据，因为我们使用GDK Pixbuf来处理
    // 只需要验证文件是有效的JPEG即可
    jpeg_destroy_decompress(&cinfo);
    fclose(fp);
    
    // 更新图像到界面
    GdkPixbuf *pixbuf = gdk_pixbuf_new_from_file(filename, NULL);
    if (pixbuf) {
        g_print("✓ 成功创建GDK Pixbuf\n");
        
        // 缩放图像到目标尺寸
        GdkPixbuf *scaled_pixbuf = scale_pixbuf_to_target(pixbuf);
        if (!scaled_pixbuf) {
            g_print("错误: 无法缩放图像\n");
            g_object_unref(pixbuf);
            return FALSE;
        }
        g_object_unref(pixbuf); // 释放原始pixbuf
        pixbuf = scaled_pixbuf;
        
        g_print("图像已缩放至: %d x %d\n", TARGET_WIDTH, TARGET_HEIGHT);
        
        // 清除之前的图像
        if (current_image) {
            g_object_unref(current_image);
        }
        
        // 设置新图像（使用原始尺寸图像）
        GdkPixbuf *original_pixbuf = gdk_pixbuf_new_from_file(filename, NULL);
        if (original_pixbuf) {
            gtk_image_set_from_pixbuf(GTK_IMAGE(image_area), original_pixbuf);
            g_object_unref(original_pixbuf);
        } else {
            gtk_image_set_from_pixbuf(GTK_IMAGE(image_area), pixbuf);
        }
        current_image = pixbuf; // 但保持缩放后的图像用于处理
        
        // 清空 左下 original数组 与 右侧 IMO 视图
        if (original_array_area)
            gtk_image_set_from_icon_name(GTK_IMAGE(original_array_area), "image-x-generic", GTK_ICON_SIZE_DIALOG);
        if (imo_image_view)
            gtk_image_set_from_icon_name(GTK_IMAGE(imo_image_view), "image-x-generic", GTK_ICON_SIZE_DIALOG);
        
        // 释放之前的二值图数据
        free_binary_image_data();
        
        // 为后续处理做好准备 - 将图像数据存储为二维uint8_t数组
        image_width = TARGET_WIDTH;
        image_height = TARGET_HEIGHT;
        
        g_print("正在分配内存: %d x %d\n", image_width, image_height);
        
        // 分配二维数组内存
        original_bi_image = (uint8_t **)malloc(image_height * sizeof(uint8_t *));
        if (!original_bi_image) {
            g_print("错误: 无法为图像数据分配内存 (第一维)\n");
            g_object_unref(pixbuf);
            current_image = NULL;
            return FALSE;
        }
        
        for (int i = 0; i < image_height; i++) {
            original_bi_image[i] = (uint8_t *)malloc(image_width * sizeof(uint8_t));
            if (!original_bi_image[i]) {
                g_print("错误: 无法为图像数据分配内存 (第%d行)\n", i);
                // 释放已分配的内存
                for (int j = 0; j < i; j++) {
                    free(original_bi_image[j]);
                }
                free(original_bi_image);
                original_bi_image = NULL;
                
                g_object_unref(pixbuf);
                current_image = NULL;
                return FALSE;
            }
        }
        g_print("✓ 内存分配成功\n");
        
        // 获取缩放后图像的像素数据
        int rowstride = gdk_pixbuf_get_rowstride(pixbuf);
        guchar *pixels = gdk_pixbuf_get_pixels(pixbuf);
        int channels = gdk_pixbuf_get_n_channels(pixbuf);
        
        // 读取图像数据并转换为二值格式
        g_print("正在读取缩放后JPEG图像数据...\n");
        int non_binary_pixels = 0;
        for (int y = 0; y < image_height; y++) {
            guchar *row = pixels + y * rowstride;
            for (int x = 0; x < image_width; x++) {
                if (channels == 1) {
                    // 灰度图像
                    int gray = row[x];
                    if (gray != 0 && gray != 255) {
                        non_binary_pixels++;
                    }
                    original_bi_image[y][x] = (gray > 128) ? 255 : 0;
                } else if (channels >= 3) {
                    // RGB彩色图像
                    int r = row[x * channels];
                    int g = row[x * channels + 1];
                    int b = row[x * channels + 2];
                    int gray = (int)(0.299 * r + 0.587 * g + 0.114 * b);
                    
                    if (gray != 0 && gray != 255) {
                        non_binary_pixels++;
                    }
                    original_bi_image[y][x] = (gray > 128) ? 255 : 0;
                }
            }
        }
        
        if (non_binary_pixels > 0) {
            g_print("警告: 发现 %d 个非二值像素，已自动转换为二值\n", non_binary_pixels);
        }
        
        g_print("✓ JPEG图像数据读取完成\n");
        g_print("✓ 图像数据转换完成\n");
        
        // 为后续处理做好准备
        g_print("图像已加载并存储为二维uint8_t数组\n");
        g_print("图像尺寸: %d x %d\n", image_width, image_height);
    } else {
        g_print("错误: 无法从文件创建GDK Pixbuf\n");
    }
    
    g_print("JPEG文件处理完成: %s\n", pixbuf ? "成功" : "失败");
    return pixbuf != NULL;
}

// ================= 渲染与自适应 =================
static GdkPixbuf* render_bw_array_pixbuf(uint8_t **arr, int w, int h, int pixel_size) {
    if (!arr || w <= 0 || h <= 0) return NULL;
    const int display_width = w * pixel_size;
    const int display_height = h * pixel_size;
    GdkPixbuf *pb = gdk_pixbuf_new(GDK_COLORSPACE_RGB, FALSE, 8, display_width, display_height);
    if (!pb) return NULL;
    gdk_pixbuf_fill(pb, 0xffffffff);
    int rowstride = gdk_pixbuf_get_rowstride(pb);
    guchar *pixels = gdk_pixbuf_get_pixels(pb);
    for (int y = 0; y < h; ++y) {
        for (int x = 0; x < w; ++x) {
            guint32 color = (arr[y][x] == 0) ? 0xff000000 : 0xffffffff; // 0=黑,255=白
            for (int dy = 0; dy < pixel_size; ++dy) {
                for (int dx = 0; dx < pixel_size; ++dx) {
                    guchar *p = pixels + (y * pixel_size + dy) * rowstride + (x * pixel_size + dx) * 3;
                    p[0] = (color >> 16) & 0xff;
                    p[1] = (color >> 8) & 0xff;
                    p[2] = color & 0xff;
                }
            }
        }
    }
    return pb;
}

static GdkPixbuf* render_imo_array_pixbuf(uint8_t **arr, int w, int h, int pixel_size) {
    if (!arr || w <= 0 || h <= 0) return NULL;
    const int display_width = w * pixel_size;
    const int display_height = h * pixel_size;
    GdkPixbuf *pb = gdk_pixbuf_new(GDK_COLORSPACE_RGB, FALSE, 8, display_width, display_height);
    if (!pb) return NULL;
    gdk_pixbuf_fill(pb, 0xffffffff);
    int rowstride = gdk_pixbuf_get_rowstride(pb);
    guchar *pixels = gdk_pixbuf_get_pixels(pb);
    for (int y = 0; y < h; ++y) {
        for (int x = 0; x < w; ++x) {
            guint32 color;
            switch (arr[y][x]) {
                case 0: color = 0xff000000; break; // 黑
                case 1: color = 0xffff0000; break; // 红
                case 2: color = 0xffffa500; break; // 橙
                case 3: color = 0xffffff00; break; // 黄
                case 4: color = 0xff00ff00; break; // 绿
                case 5: color = 0xff00ffff; break; // 青
                case 255: color = 0xffffffff; break; // 白
                default: color = 0xffffffff; break; // 其他->白
            }
            for (int dy = 0; dy < pixel_size; ++dy) {
                for (int dx = 0; dx < pixel_size; ++dx) {
                    guchar *p = pixels + (y * pixel_size + dy) * rowstride + (x * pixel_size + dx) * 3;
                    p[0] = (color >> 16) & 0xff;
                    p[1] = (color >> 8) & 0xff;
                    p[2] = color & 0xff;
                }
            }
        }
    }
    return pb;
}

static void refresh_all_views() {
    // 左下：original 数组图
    if (original_bi_image && original_array_area) {
        GdkPixbuf *base = render_bw_array_pixbuf(original_bi_image, image_width, image_height, 2);
        if (base) {
            GtkAllocation alloc;
            gtk_widget_get_allocation(original_array_area, &alloc);
            int target_w = alloc.width > 8 ? alloc.width - 8 : alloc.width;
            int target_h = alloc.height > 8 ? alloc.height - 8 : alloc.height;
            if (target_w > 0 && target_h > 0) {
                double sx = (double)target_w / gdk_pixbuf_get_width(base);
                double sy = (double)target_h / gdk_pixbuf_get_height(base);
                double s = sx < sy ? sx : sy; // 等比缩放
                int nw = (int)(gdk_pixbuf_get_width(base) * s);
                int nh = (int)(gdk_pixbuf_get_height(base) * s);
                if (nw <= 0) nw = 1;
                if (nh <= 0) nh = 1;
                GdkPixbuf *scaled = gdk_pixbuf_scale_simple(base, nw, nh, GDK_INTERP_BILINEAR);
                if (scaled) {
                    gtk_image_set_from_pixbuf(GTK_IMAGE(original_array_area), scaled);
                    g_object_unref(scaled);
                } else {
                    gtk_image_set_from_pixbuf(GTK_IMAGE(original_array_area), base);
                }
            } else {
                gtk_image_set_from_pixbuf(GTK_IMAGE(original_array_area), base);
            }
            g_object_unref(base);
        }
    } else if (original_array_area) {
        // 没有数据时显示占位，避免残留
        gtk_image_set_from_icon_name(GTK_IMAGE(original_array_area), "image-x-generic", GTK_ICON_SIZE_DIALOG);
    }

    // 右侧：IMO 图
    if (imo && imo_image_view) {
        GdkPixbuf *base = render_imo_array_pixbuf(imo, image_width, image_height, 2);
        if (base) {
            GtkAllocation alloc;
            gtk_widget_get_allocation(imo_image_view, &alloc);
            int target_w = alloc.width > 8 ? alloc.width - 8 : alloc.width;
            int target_h = alloc.height > 8 ? alloc.height - 8 : alloc.height;
            if (target_w > 0 && target_h > 0) {
                double sx = (double)target_w / gdk_pixbuf_get_width(base);
                double sy = (double)target_h / gdk_pixbuf_get_height(base);
                double s = sx < sy ? sx : sy;
                int nw = (int)(gdk_pixbuf_get_width(base) * s);
                int nh = (int)(gdk_pixbuf_get_height(base) * s);
                if (nw <= 0) nw = 1;
                if (nh <= 0) nh = 1;
                GdkPixbuf *scaled = gdk_pixbuf_scale_simple(base, nw, nh, GDK_INTERP_BILINEAR);
                if (scaled) {
                    gtk_image_set_from_pixbuf(GTK_IMAGE(imo_image_view), scaled);
                    g_object_unref(scaled);
                } else {
                    gtk_image_set_from_pixbuf(GTK_IMAGE(imo_image_view), base);
                }
            } else {
                gtk_image_set_from_pixbuf(GTK_IMAGE(imo_image_view), base);
            }
            g_object_unref(base);
        }
    } else if (imo_image_view) {
        // 没有数据时显示占位，避免残留
        gtk_image_set_from_icon_name(GTK_IMAGE(imo_image_view), "image-x-generic", GTK_ICON_SIZE_DIALOG);
    }
}

static void on_size_allocate(GtkWidget *widget, GdkRectangle *allocation, gpointer user_data) {
    (void)widget; (void)allocation; (void)user_data;
    refresh_all_views();
}