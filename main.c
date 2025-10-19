#include <gtk/gtk.h>
#include <png.h>
#include <jpeglib.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

// 全局变量
GtkWidget *window;
GtkWidget *image_area;
GtkWidget *original_image_area;  // 用于显示imo数组
GtkWidget *upload_button;
GtkWidget *show_array_button;     // 保留查看数组按钮
GtkWidget *show_imo_button;      // 添加显示imo按钮
GtkWidget *process_button;       // 添加处理按钮
GtkWidget *array_window = NULL;
GtkWidget *array_image_view = NULL;
GtkWidget *imo_window = NULL;    // 添加imo显示窗口
GtkWidget *imo_image_view = NULL; // 添加imo图像视图
GdkPixbuf *current_image = NULL;

// 用于存储二值图数据的二维数组
uint8_t **original_bi_image = NULL;
uint8_t **imo = NULL;            // 添加最终输出数组
int image_width = 0;
int image_height = 0;

// 目标尺寸 (120行 x 188列)
const int TARGET_WIDTH = 188;
const int TARGET_HEIGHT = 120;

// 函数声明
static void activate(GtkApplication *app, gpointer user_data);
static void upload_image_clicked(GtkWidget *widget, gpointer data);
static void show_array_clicked(GtkWidget *widget, gpointer data);
static void show_imo_clicked(GtkWidget *widget, gpointer data);     // 添加imo显示函数声明
static void process_image_clicked(GtkWidget *widget, gpointer data); // 添加处理函数声明
static void array_window_closed(GtkWidget *widget, gpointer data);
static void imo_window_closed(GtkWidget *widget, gpointer data);    // 添加imo窗口关闭函数声明
static gboolean load_png_image(const gchar *filename);
static gboolean load_jpeg_image(const gchar *filename);
static gboolean is_jpeg_file(const gchar *filename);
static gboolean is_png_file(const gchar *filename);
static void free_binary_image_data();
static GdkPixbuf* scale_pixbuf_to_target(GdkPixbuf *pixbuf);
static void allocate_imo_array();                                   // 添加分配imo数组函数声明

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
    
    // 创建显示数组按钮
    show_array_button = gtk_button_new_with_label("查看原始数组");
    g_signal_connect(show_array_button, "clicked", G_CALLBACK(show_array_clicked), NULL);
    gtk_box_pack_start(GTK_BOX(hbox), show_array_button, FALSE, FALSE, 0);
    
    // 创建显示imo按钮
    show_imo_button = gtk_button_new_with_label("查看IMO数组");
    g_signal_connect(show_imo_button, "clicked", G_CALLBACK(show_imo_clicked), NULL);
    gtk_box_pack_start(GTK_BOX(hbox), show_imo_button, FALSE, FALSE, 0);
    
    // 默认禁用所有按钮
    gtk_widget_set_sensitive(process_button, FALSE);
    gtk_widget_set_sensitive(show_array_button, FALSE);
    gtk_widget_set_sensitive(show_imo_button, FALSE);

    // 创建水平布局用于显示图像
    GtkWidget *image_hbox = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 10);
    gtk_box_set_homogeneous(GTK_BOX(image_hbox), TRUE); // 均匀分配空间
    gtk_box_pack_start(GTK_BOX(vbox), image_hbox, TRUE, TRUE, 0);

    // 创建当前图像显示区域（带滚动条）
    GtkWidget *current_frame = gtk_frame_new("当前图像 (原始尺寸)");
    gtk_frame_set_shadow_type(GTK_FRAME(current_frame), GTK_SHADOW_ETCHED_IN);
    GtkWidget *current_scroll = gtk_scrolled_window_new(NULL, NULL);
    gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(current_scroll),
                                   GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
    gtk_container_add(GTK_CONTAINER(current_frame), current_scroll);
    gtk_box_pack_start(GTK_BOX(image_hbox), current_frame, TRUE, TRUE, 0);
    
    // 图像显示区域
    image_area = gtk_image_new();
    gtk_container_add(GTK_CONTAINER(current_scroll), image_area);

    // 创建imo显示区域（带滚动条）
    GtkWidget *imo_frame = gtk_frame_new("IMO数组显示");
    gtk_frame_set_shadow_type(GTK_FRAME(imo_frame), GTK_SHADOW_ETCHED_IN);
    GtkWidget *imo_scroll = gtk_scrolled_window_new(NULL, NULL);
    gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(imo_scroll),
                                   GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
    gtk_container_add(GTK_CONTAINER(imo_frame), imo_scroll);
    gtk_box_pack_start(GTK_BOX(image_hbox), imo_frame, TRUE, TRUE, 0);
    
    // imo显示区域
    original_image_area = gtk_image_new();
    gtk_container_add(GTK_CONTAINER(imo_scroll), original_image_area);

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
            gtk_widget_set_sensitive(show_array_button, TRUE);
            gtk_widget_set_sensitive(show_imo_button, FALSE); // IMO数组尚未生成
        }
        
        g_free(filename);
    }

    gtk_widget_destroy(dialog);
}

// 显示数组按钮点击事件处理函数
static void show_array_clicked(GtkWidget *widget, gpointer data) {
    if (!original_bi_image || image_width <= 0 || image_height <= 0) {
        return;
    }

    // 如果数组窗口已经打开，将其提到前面
    if (array_window) {
        gtk_window_present(GTK_WINDOW(array_window));
        return;
    }

    // 创建新的数组查看窗口
    array_window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_window_set_title(GTK_WINDOW(array_window), "原始数组数据 (120行×188列)");
    gtk_window_set_default_size(GTK_WINDOW(array_window), 800, 600);
    gtk_window_set_transient_for(GTK_WINDOW(array_window), GTK_WINDOW(window));
    
    // 连接窗口关闭信号
    g_signal_connect(array_window, "destroy", G_CALLBACK(array_window_closed), NULL);

    // 创建滚动窗口
    GtkWidget *scroll = gtk_scrolled_window_new(NULL, NULL);
    gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(scroll),
                                   GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
    gtk_container_add(GTK_CONTAINER(array_window), scroll);
    
    // 创建图像视图而不是文本视图
    array_image_view = gtk_image_new();
    gtk_container_add(GTK_CONTAINER(scroll), array_image_view);
    
    // 创建一个足够大的pixbuf来显示完整的数组
    // 每个像素点用4x4的方块来表示，这样整个图像就是 188*4 x 120*4 = 752 x 480 像素
    const int pixel_size = 4;
    const int display_width = TARGET_WIDTH * pixel_size;
    const int display_height = TARGET_HEIGHT * pixel_size;
    
    GdkPixbuf *array_pixbuf = gdk_pixbuf_new(GDK_COLORSPACE_RGB, FALSE, 8, 
                                            display_width, display_height);
    
    if (array_pixbuf) {
        // 填充背景为白色
        gdk_pixbuf_fill(array_pixbuf, 0xffffffff); // 白色背景
        
        // 获取pixbuf信息
        int rowstride = gdk_pixbuf_get_rowstride(array_pixbuf);
        guchar *pixels = gdk_pixbuf_get_pixels(array_pixbuf);
        
        // 绘制数组数据
        for (int y = 0; y < image_height; y++) {
            for (int x = 0; x < image_width; x++) {
                // 计算在显示图像中的位置
                int display_x = x * pixel_size;
                int display_y = y * pixel_size;
                
                // 获取颜色值 (0=黑色, 1=白色)
                guint32 color = (original_bi_image[y][x] == 1) ? 0xffffffff : 0xff000000;
                
                // 绘制pixel_size x pixel_size的方块
                for (int dy = 0; dy < pixel_size; dy++) {
                    for (int dx = 0; dx < pixel_size; dx++) {
                        if (display_y + dy < display_height && display_x + dx < display_width) {
                            guchar *pixel = pixels + (display_y + dy) * rowstride + (display_x + dx) * 3;
                            pixel[0] = (color >> 16) & 0xff; // R
                            pixel[1] = (color >> 8) & 0xff;  // G
                            pixel[2] = color & 0xff;         // B
                        }
                    }
                }
            }
        }
        
        // 设置图像
        gtk_image_set_from_pixbuf(GTK_IMAGE(array_image_view), array_pixbuf);
        g_object_unref(array_pixbuf);
    }
    
    gtk_widget_show_all(array_window);
}

// 数组窗口关闭回调函数
static void array_window_closed(GtkWidget *widget, gpointer data) {
    array_window = NULL;
    array_image_view = NULL;
}

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
        
        // 清空原图显示
        gtk_image_set_from_icon_name(GTK_IMAGE(original_image_area), "image-missing", GTK_ICON_SIZE_DIALOG);
        
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
                    original_bi_image[y][x] = (gray > 128) ? 1 : 0;
                } else if (channels == 2) {
                    // 灰度+Alpha图像
                    int gray = row[x * 2];
                    int alpha = row[x * 2 + 1];
                    
                    // 如果alpha为0，则为透明（这里当作白色处理）
                    if (alpha == 0) {
                        original_bi_image[y][x] = 1; // 白色
                    } else {
                        if (gray != 0 && gray != 255) {
                            non_binary_pixels++;
                        }
                        original_bi_image[y][x] = (gray > 128) ? 1 : 0;
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
                    original_bi_image[y][x] = (gray > 128) ? 1 : 0;
                } else if (channels == 4) {
                    // RGBA图像
                    int r = row[x * 4];
                    int g = row[x * 4 + 1];
                    int b = row[x * 4 + 2];
                    int a = row[x * 4 + 3];
                    
                    // 如果alpha为0，则为透明（这里当作白色处理）
                    if (a == 0) {
                        original_bi_image[y][x] = 1; // 白色
                    } else {
                        // RGB转灰度
                        int gray = (int)(0.299 * r + 0.587 * g + 0.114 * b);
                        
                        // 检查灰度值是否为二值
                        if (gray != 0 && gray != 255) {
                            non_binary_pixels++;
                        }
                        
                        // 转换为二值：0或1
                        original_bi_image[y][x] = (gray > 128) ? 1 : 0;
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
    
    // 示例处理：将original_bi_image复制到imo（用户可以在这里实现自己的处理逻辑）
    for (int y = 0; y < image_height; y++) {
        for (int x = 0; x < image_width; x++) {
            imo[y][x] = original_bi_image[y][x];
        }
    }
    
    // 为了测试彩色显示功能，将一些特定区域的值设置为1-5
    // 在图像顶部添加彩色测试行
    if (image_height > 5) {
        for (int x = 0; x < image_width && x < 30; x++) {
            imo[0][x] = 1; // 红色
            if (x < 25) imo[1][x] = 2; // 橙色
            if (x < 20) imo[2][x] = 3; // 黄色
            if (x < 15) imo[3][x] = 4; // 绿色
            if (x < 10) imo[4][x] = 5; // 青色
        }
    }
    
    g_print("图像处理完成，imo数组已生成\n");
    
    // 启用显示imo按钮
    gtk_widget_set_sensitive(show_imo_button, TRUE);
}

// 显示imo按钮点击事件处理函数
static void show_imo_clicked(GtkWidget *widget, gpointer data) {
    if (!imo || image_width <= 0 || image_height <= 0) {
        return;
    }

    // 如果imo窗口已经打开，将其提到前面
    if (imo_window) {
        gtk_window_present(GTK_WINDOW(imo_window));
        return;
    }

    // 创建新的imo查看窗口
    imo_window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_window_set_title(GTK_WINDOW(imo_window), "IMO数组数据 (120行×188列)");
    gtk_window_set_default_size(GTK_WINDOW(imo_window), 800, 600);
    gtk_window_set_transient_for(GTK_WINDOW(imo_window), GTK_WINDOW(window));
    
    // 连接窗口关闭信号
    g_signal_connect(imo_window, "destroy", G_CALLBACK(imo_window_closed), NULL);

    // 创建滚动窗口
    GtkWidget *scroll = gtk_scrolled_window_new(NULL, NULL);
    gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(scroll),
                                   GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
    gtk_container_add(GTK_CONTAINER(imo_window), scroll);
    
    // 创建图像视图
    imo_image_view = gtk_image_new();
    gtk_container_add(GTK_CONTAINER(scroll), imo_image_view);
    
    // 创建一个足够大的pixbuf来显示完整的数组
    // 每个像素点用4x4的方块来表示，这样整个图像就是 188*4 x 120*4 = 752 x 480 像素
    const int pixel_size = 4;
    const int display_width = TARGET_WIDTH * pixel_size;
    const int display_height = TARGET_HEIGHT * pixel_size;
    
    GdkPixbuf *imo_pixbuf = gdk_pixbuf_new(GDK_COLORSPACE_RGB, FALSE, 8, 
                                          display_width, display_height);
    
    if (imo_pixbuf) {
        // 填充背景为白色
        gdk_pixbuf_fill(imo_pixbuf, 0xffffffff); // 白色背景
        
        // 获取pixbuf信息
        int rowstride = gdk_pixbuf_get_rowstride(imo_pixbuf);
        guchar *pixels = gdk_pixbuf_get_pixels(imo_pixbuf);
        
        // 绘制数组数据
        for (int y = 0; y < image_height; y++) {
            for (int x = 0; x < image_width; x++) {
                // 计算在显示图像中的位置
                int display_x = x * pixel_size;
                int display_y = y * pixel_size;
                
                // 获取颜色值，根据imo数组中的值确定颜色
                guint32 color;
                switch (imo[y][x]) {
                    case 0:
                        color = 0xff000000; // 黑色
                        break;
                    case 1:
                        color = 0xffff0000; // 红色
                        break;
                    case 2:
                        color = 0xffffa500; // 橙色
                        break;
                    case 3:
                        color = 0xffffff00; // 黄色
                        break;
                    case 4:
                        color = 0xff00ff00; // 绿色
                        break;
                    case 5:
                        color = 0xff00ffff; // 青色
                        break;
                    default:
                        // 其他值显示为白色
                        color = 0xffffffff; // 白色
                        break;
                }
                
                // 绘制pixel_size x pixel_size的方块
                for (int dy = 0; dy < pixel_size; dy++) {
                    for (int dx = 0; dx < pixel_size; dx++) {
                        if (display_y + dy < display_height && display_x + dx < display_width) {
                            guchar *pixel = pixels + (display_y + dy) * rowstride + (display_x + dx) * 3;
                            pixel[0] = (color >> 16) & 0xff; // R
                            pixel[1] = (color >> 8) & 0xff;  // G
                            pixel[2] = color & 0xff;         // B
                        }
                    }
                }
            }
        }
        
        // 设置图像
        gtk_image_set_from_pixbuf(GTK_IMAGE(imo_image_view), imo_pixbuf);
        g_object_unref(imo_pixbuf);
    }
    
    gtk_widget_show_all(imo_window);
}

// imo窗口关闭回调函数
static void imo_window_closed(GtkWidget *widget, gpointer data) {
    imo_window = NULL;
    imo_image_view = NULL;
}

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
        
        // 清空原图显示
        gtk_image_set_from_icon_name(GTK_IMAGE(original_image_area), "image-missing", GTK_ICON_SIZE_DIALOG);
        
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
                    original_bi_image[y][x] = (gray > 128) ? 1 : 0;
                } else if (channels >= 3) {
                    // RGB彩色图像
                    int r = row[x * channels];
                    int g = row[x * channels + 1];
                    int b = row[x * channels + 2];
                    int gray = (int)(0.299 * r + 0.587 * g + 0.114 * b);
                    
                    if (gray != 0 && gray != 255) {
                        non_binary_pixels++;
                    }
                    original_bi_image[y][x] = (gray > 128) ? 1 : 0;
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