#include <gtk/gtk.h>
#include <png.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include "processor.h"
#include "global_image_buffer.h"

#ifdef HAVE_OPENCV
// 兼容 MSYS2 mingw64 (msvcrt) 缺少 quick_exit/timespec_get 的环境：
// 仅做前置声明以满足 <cstdlib>/<ctime> 中的 using 声明，代码中不实际调用这些符号。
extern "C" {
    void quick_exit(int);
    int at_quick_exit(void (*)(void));
    struct timespec; int timespec_get(struct timespec*, int);
}
#include <opencv2/opencv.hpp>
#include <string>
#include <vector>
#endif

// 复用原有全局
static GtkWidget *window;
static GtkWidget *image_area;
static GtkWidget *original_array_area;
static GtkWidget *process_button;
static GtkWidget *imo_image_view;
static GdkPixbuf *current_image = NULL;

// 全局二维数组已在 global_image_buffer.h/c 定义
static int image_width = IMAGE_W;
static int image_height = IMAGE_H;
static const int TARGET_WIDTH = IMAGE_W;
static const int TARGET_HEIGHT = IMAGE_H;

#ifdef HAVE_OPENCV
// 视频相关状态
static cv::VideoCapture g_cap;
static int g_frame_index = 0;
static std::string g_video_path;
#endif

// 前置声明
static void activate(GtkApplication *app, gpointer user_data);
static void reset_image_views();
static void free_binary_image_data();
static void allocate_imo_array();
static void refresh_all_views();
static void on_size_allocate(GtkWidget *widget, GdkRectangle *allocation, gpointer user_data);
static GdkPixbuf* scale_pixbuf_to_target(GdkPixbuf *pixbuf);

// 从 pixbuf 填充 original_bi_image
static gboolean pixbuf_to_binary_array(GdkPixbuf *pixbuf);

// 载图按钮
static void upload_image_clicked(GtkWidget *widget, gpointer data);
static gboolean load_png_image(const gchar *filename);
static gboolean load_jpeg_image(const gchar *filename);
static gboolean is_png_file(const gchar *filename);
static gboolean is_jpeg_file(const gchar *filename);
static GdkPixbuf* render_bw_array_pixbuf(uint8_t **arr, int w, int h, int pixel_size);
static GdkPixbuf* render_imo_array_pixbuf(uint8_t **arr, int w, int h, int pixel_size);
static void process_image_clicked(GtkWidget *widget, gpointer data);

#ifdef HAVE_OPENCV
// 视频导入相关回调
static void open_video_clicked(GtkWidget *widget, gpointer data);
static void next_frame_clicked(GtkWidget *widget, gpointer data);
static void prev_frame_clicked(GtkWidget *widget, gpointer data);
static void show_cv_frame(const cv::Mat &frame);
static void open_tiled_selector_clicked(GtkWidget *widget, gpointer data);
#endif

int main(int argc, char **argv) {
    GtkApplication *app;
    int status;
    app = gtk_application_new("com.example.binaryimage", G_APPLICATION_DEFAULT_FLAGS);
    g_signal_connect(app, "activate", G_CALLBACK(activate), NULL);
    status = g_application_run(G_APPLICATION(app), argc, argv);
    g_object_unref(app);

    free_binary_image_data();
    return status;
}

static void activate(GtkApplication *app, gpointer user_data) {
    // 初始化全局缓冲，默认白底，避免初始全黑影响观感
    init_global_image_buffers_default();
    window = gtk_application_window_new(app);
    gtk_window_set_title(GTK_WINDOW(window), "二值图处理器（含视频）");
    gtk_window_set_default_size(GTK_WINDOW(window), 1100, 720);
    gtk_window_set_position(GTK_WINDOW(window), GTK_WIN_POS_CENTER);

    GtkWidget *vbox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 10);
    gtk_container_set_border_width(GTK_CONTAINER(vbox), 10);
    gtk_container_add(GTK_CONTAINER(window), vbox);

    GtkWidget *hbox = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 8);
    gtk_box_pack_start(GTK_BOX(vbox), hbox, FALSE, FALSE, 0);

    GtkWidget *upload_button = gtk_button_new_with_label("上传PNG/JPEG二值图");
    g_signal_connect(upload_button, "clicked", G_CALLBACK(upload_image_clicked), NULL);
    gtk_box_pack_start(GTK_BOX(hbox), upload_button, FALSE, FALSE, 0);

    process_button = gtk_button_new_with_label("处理图像 (original->imo)");
    g_signal_connect(process_button, "clicked", G_CALLBACK(process_image_clicked), NULL);
    gtk_box_pack_start(GTK_BOX(hbox), process_button, FALSE, FALSE, 0);
    gtk_widget_set_sensitive(process_button, FALSE);

#ifdef HAVE_OPENCV
    GtkWidget *open_video_btn = gtk_button_new_with_label("导入视频(mp4)");
    gtk_box_pack_start(GTK_BOX(hbox), open_video_btn, FALSE, FALSE, 0);
    g_signal_connect(open_video_btn, "clicked", G_CALLBACK(open_video_clicked), NULL);

    GtkWidget *prev_btn = gtk_button_new_with_label("上一帧");
    gtk_box_pack_start(GTK_BOX(hbox), prev_btn, FALSE, FALSE, 0);
    g_signal_connect(prev_btn, "clicked", G_CALLBACK(prev_frame_clicked), NULL);

    GtkWidget *next_btn = gtk_button_new_with_label("下一帧");
    gtk_box_pack_start(GTK_BOX(hbox), next_btn, FALSE, FALSE, 0);
    g_signal_connect(next_btn, "clicked", G_CALLBACK(next_frame_clicked), NULL);

    GtkWidget *tiled_btn = gtk_button_new_with_label("平铺选帧");
    gtk_box_pack_start(GTK_BOX(hbox), tiled_btn, FALSE, FALSE, 0);
    g_signal_connect(tiled_btn, "clicked", G_CALLBACK(open_tiled_selector_clicked), NULL);
#endif

    GtkWidget *image_hbox = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 10);
    gtk_box_pack_start(GTK_BOX(vbox), image_hbox, TRUE, TRUE, 0);

    GtkWidget *left_vbox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 10);
    gtk_box_pack_start(GTK_BOX(image_hbox), left_vbox, TRUE, TRUE, 0);

    GtkWidget *current_frame = gtk_frame_new("原始输入图");
    gtk_frame_set_shadow_type(GTK_FRAME(current_frame), GTK_SHADOW_ETCHED_IN);
    image_area = gtk_image_new();
    gtk_container_add(GTK_CONTAINER(current_frame), image_area);
    gtk_box_pack_start(GTK_BOX(left_vbox), current_frame, TRUE, TRUE, 0);

    GtkWidget *orig_frame = gtk_frame_new("Original 数组图 (0/255)");
    gtk_frame_set_shadow_type(GTK_FRAME(orig_frame), GTK_SHADOW_ETCHED_IN);
    original_array_area = gtk_image_new();
    gtk_container_add(GTK_CONTAINER(orig_frame), original_array_area);
    gtk_box_pack_start(GTK_BOX(left_vbox), orig_frame, TRUE, TRUE, 0);

    GtkWidget *imo_frame = gtk_frame_new("IMO 数组图");
    gtk_frame_set_shadow_type(GTK_FRAME(imo_frame), GTK_SHADOW_ETCHED_IN);
    imo_image_view = gtk_image_new();
    gtk_container_add(GTK_CONTAINER(imo_frame), imo_image_view);
    gtk_box_pack_start(GTK_BOX(image_hbox), imo_frame, TRUE, TRUE, 0);

    g_signal_connect(current_frame, "size-allocate", G_CALLBACK(on_size_allocate), NULL);
    g_signal_connect(orig_frame, "size-allocate", G_CALLBACK(on_size_allocate), NULL);
    g_signal_connect(imo_frame, "size-allocate", G_CALLBACK(on_size_allocate), NULL);

    reset_image_views();
    gtk_widget_show_all(window);
}

// 全局数组无需释放
static void free_imo_array() {}
static void free_binary_image_data() {}

static void reset_image_views() {
    if (original_array_area)
        gtk_image_set_from_icon_name(GTK_IMAGE(original_array_area), "image-x-generic", GTK_ICON_SIZE_DIALOG);
    if (imo_image_view)
        gtk_image_set_from_icon_name(GTK_IMAGE(imo_image_view), "image-x-generic", GTK_ICON_SIZE_DIALOG);
}

static GdkPixbuf* scale_pixbuf_to_target(GdkPixbuf *pixbuf) {
    if (!pixbuf) return NULL;
    int width = gdk_pixbuf_get_width(pixbuf);
    int height = gdk_pixbuf_get_height(pixbuf);
    if (width == TARGET_WIDTH && height == TARGET_HEIGHT) return g_object_ref(pixbuf);
    return gdk_pixbuf_scale_simple(pixbuf, TARGET_WIDTH, TARGET_HEIGHT, GDK_INTERP_BILINEAR);
}

static gboolean pixbuf_to_binary_array(GdkPixbuf *pixbuf) {
    if (!pixbuf) return FALSE;
    int rowstride = gdk_pixbuf_get_rowstride(pixbuf);
    guchar *pixels = gdk_pixbuf_get_pixels(pixbuf);
    int channels = gdk_pixbuf_get_n_channels(pixbuf);
    for (int y = 0; y < IMAGE_H; y++) {
        guchar *row = pixels + y * rowstride;
        for (int x = 0; x < IMAGE_W; x++) {
            int gray;
            if (channels == 1) gray = row[x];
            else if (channels == 2) gray = row[x * 2];
            else if (channels == 3) {
                int r = row[x * 3]; int g = row[x * 3 + 1]; int b = row[x * 3 + 2];
                gray = (int)(0.299 * r + 0.587 * g + 0.114 * b);
            } else {
                int r = row[x * 4]; int g = row[x * 4 + 1]; int b = row[x * 4 + 2]; int a = row[x * 4 + 3];
                if (a == 0) { original_bi_image[y][x] = 255; continue; }
                gray = (int)(0.299 * r + 0.587 * g + 0.114 * b);
            }
            original_bi_image[y][x] = (gray > 128) ? 255 : 0;
        }
    }
    return TRUE;
}

static gboolean is_png_file(const gchar *filename) {
    FILE *fp = fopen(filename, "rb");
    if (!fp) return FALSE;
    png_byte header[8]; fread(header, 1, 8, fp); fclose(fp);
    return (png_sig_cmp(header, 0, 8) == 0);
}

static gboolean is_jpeg_file(const gchar *filename) {
    FILE *fp = fopen(filename, "rb");
    if (!fp) return FALSE;
    unsigned char header[2]; fread(header, 1, 2, fp); fclose(fp);
    return (header[0] == 0xFF && header[1] == 0xD8);
}

static gboolean load_png_image(const gchar *filename) {
    GdkPixbuf *pixbuf = gdk_pixbuf_new_from_file(filename, NULL);
    if (!pixbuf) return FALSE;
    // 左侧显示原图
    gtk_image_set_from_pixbuf(GTK_IMAGE(image_area), pixbuf);
    // 转为 188x120
    GdkPixbuf *scaled = scale_pixbuf_to_target(pixbuf);
    g_object_unref(pixbuf);
    if (!scaled) return FALSE;
    gboolean ok = pixbuf_to_binary_array(scaled);
    g_object_unref(scaled);
    if (ok) { refresh_all_views(); gtk_widget_set_sensitive(process_button, TRUE); }
    return ok;
}

static gboolean load_jpeg_image(const gchar *filename) {
    GdkPixbuf *pixbuf = gdk_pixbuf_new_from_file(filename, NULL);
    if (!pixbuf) return FALSE;
    gtk_image_set_from_pixbuf(GTK_IMAGE(image_area), pixbuf);
    GdkPixbuf *scaled = scale_pixbuf_to_target(pixbuf);
    g_object_unref(pixbuf);
    if (!scaled) return FALSE;
    gboolean ok = pixbuf_to_binary_array(scaled);
    g_object_unref(scaled);
    if (ok) { refresh_all_views(); gtk_widget_set_sensitive(process_button, TRUE); }
    return ok;
}

static void upload_image_clicked(GtkWidget *widget, gpointer data) {
    GtkWidget *dialog;
    dialog = gtk_file_chooser_dialog_new("选择图像文件", GTK_WINDOW(window), GTK_FILE_CHOOSER_ACTION_OPEN,
                                         "_取消", GTK_RESPONSE_CANCEL, "_打开", GTK_RESPONSE_ACCEPT, NULL);
    GtkFileFilter *png_filter = gtk_file_filter_new();
    gtk_file_filter_set_name(png_filter, "PNG 文件");
    gtk_file_filter_add_pattern(png_filter, "*.png");
    gtk_file_chooser_add_filter(GTK_FILE_CHOOSER(dialog), png_filter);
    GtkFileFilter *jpeg_filter = gtk_file_filter_new();
    gtk_file_filter_set_name(jpeg_filter, "JPEG 文件");
    gtk_file_filter_add_pattern(jpeg_filter, "*.jpg");
    gtk_file_filter_add_pattern(jpeg_filter, "*.jpeg");
    gtk_file_chooser_add_filter(GTK_FILE_CHOOSER(dialog), jpeg_filter);

    if (gtk_dialog_run(GTK_DIALOG(dialog)) == GTK_RESPONSE_ACCEPT) {
        char *filename = gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(dialog));
        gboolean loaded = FALSE;
        if (is_png_file(filename)) loaded = load_png_image(filename);
        else if (is_jpeg_file(filename)) loaded = load_jpeg_image(filename);
        else {
            GtkWidget *err = gtk_message_dialog_new(GTK_WINDOW(window), GTK_DIALOG_DESTROY_WITH_PARENT,
                                                    GTK_MESSAGE_ERROR, GTK_BUTTONS_CLOSE,
                                                    "不支持的图像格式，请选择 PNG 或 JPEG");
            gtk_dialog_run(GTK_DIALOG(err)); gtk_widget_destroy(err);
        }
        if (loaded) reset_image_views(), refresh_all_views();
        g_free(filename);
    }
    gtk_widget_destroy(dialog);
}

static void allocate_imo_array() {
    for (int i = 0; i < IMAGE_H; i++) {
        for (int j = 0; j < IMAGE_W; j++) {
            imo[i][j] = 255;
        }
    }
}

static void process_image_clicked(GtkWidget *widget, gpointer data) {
    allocate_imo_array();
    process_original_to_imo(&original_bi_image[0][0], &imo[0][0], IMAGE_W, IMAGE_H);
    refresh_all_views();
}

static GdkPixbuf* render_bw_array_pixbuf(uint8_t arr[IMAGE_H][IMAGE_W], int w, int h, int pixel_size) {
    if (!arr || w <= 0 || h <= 0) return NULL;
    int W = w * pixel_size, H = h * pixel_size;
    GdkPixbuf *pb = gdk_pixbuf_new(GDK_COLORSPACE_RGB, FALSE, 8, W, H);
    if (!pb) return NULL;
    gdk_pixbuf_fill(pb, 0xffffffff);
    int rowstride = gdk_pixbuf_get_rowstride(pb);
    guchar *pixels = gdk_pixbuf_get_pixels(pb);
    for (int y = 0; y < h; ++y) {
        for (int x = 0; x < w; ++x) {
            guint32 color = (arr[y][x] == 0) ? 0xff000000 : 0xffffffff;
            for (int dy = 0; dy < pixel_size; ++dy)
                for (int dx = 0; dx < pixel_size; ++dx) {
                    guchar *p = pixels + (y * pixel_size + dy) * rowstride + (x * pixel_size + dx) * 3;
                    p[0] = (color >> 16) & 0xff; p[1] = (color >> 8) & 0xff; p[2] = color & 0xff;
                }
        }
    }
    return pb;
}

static GdkPixbuf* render_imo_array_pixbuf(uint8_t arr[IMAGE_H][IMAGE_W], int w, int h, int pixel_size) {
    if (!arr || w <= 0 || h <= 0) return NULL;
    int W = w * pixel_size, H = h * pixel_size;
    GdkPixbuf *pb = gdk_pixbuf_new(GDK_COLORSPACE_RGB, FALSE, 8, W, H);
    if (!pb) return NULL;
    gdk_pixbuf_fill(pb, 0xffffffff);
    int rowstride = gdk_pixbuf_get_rowstride(pb);
    guchar *pixels = gdk_pixbuf_get_pixels(pb);
    for (int y = 0; y < h; ++y) {
        for (int x = 0; x < w; ++x) {
            guint32 color;
            switch (arr[y][x]) {
                case 0: color = 0xff000000; break;
                case 1: color = 0xffff0000; break;
                case 2: color = 0xffffa500; break;
                case 3: color = 0xffffff00; break;
                case 4: color = 0xff00ff00; break;
                case 5: color = 0xff00ffff; break;
                case 255: default: color = 0xffffffff; break;
            }
            for (int dy = 0; dy < pixel_size; ++dy)
                for (int dx = 0; dx < pixel_size; ++dx) {
                    guchar *p = pixels + (y * pixel_size + dy) * rowstride + (x * pixel_size + dx) * 3;
                    p[0] = (color >> 16) & 0xff; p[1] = (color >> 8) & 0xff; p[2] = color & 0xff;
                }
        }
    }
    return pb;
}

static void refresh_all_views() {
    if (original_array_area) {
        GdkPixbuf *base = render_bw_array_pixbuf(original_bi_image, IMAGE_W, IMAGE_H, 2);
        if (base) { gtk_image_set_from_pixbuf(GTK_IMAGE(original_array_area), base); g_object_unref(base);}    }
    if (imo_image_view) {
        GdkPixbuf *base = render_imo_array_pixbuf(imo, IMAGE_W, IMAGE_H, 2);
        if (base) { gtk_image_set_from_pixbuf(GTK_IMAGE(imo_image_view), base); g_object_unref(base);}    }
}

static void on_size_allocate(GtkWidget *widget, GdkRectangle *allocation, gpointer user_data) {
    (void)widget; (void)allocation; (void)user_data; refresh_all_views(); }

#ifdef HAVE_OPENCV
static GdkPixbuf* pixbuf_from_rgb_mat_copy(const cv::Mat &rgb)
{
    GdkPixbuf *pb = gdk_pixbuf_new(GDK_COLORSPACE_RGB, FALSE, 8, rgb.cols, rgb.rows);
    if (!pb) return NULL;
    int rowstride = gdk_pixbuf_get_rowstride(pb);
    guchar *pixels = gdk_pixbuf_get_pixels(pb);
    for (int y = 0; y < rgb.rows; ++y) {
        memcpy(pixels + y * rowstride, rgb.ptr(y), rgb.cols * 3);
    }
    return pb;
}

static void show_cv_frame(const cv::Mat &frame) {
    // 左侧显示原始帧（安全复制到 GdkPixbuf）
    cv::Mat bgr; if (frame.channels()==3) bgr = frame; else cv::cvtColor(frame, bgr, cv::COLOR_GRAY2BGR);
    cv::Mat rgb; cv::cvtColor(bgr, rgb, cv::COLOR_BGR2RGB);
    GdkPixbuf *pb = pixbuf_from_rgb_mat_copy(rgb);
    if (pb) {
        gtk_image_set_from_pixbuf(GTK_IMAGE(image_area), pb);
        // 生成 188x120 的 pixbuf 用于二值化
        GdkPixbuf *scaled = gdk_pixbuf_scale_simple(pb, TARGET_WIDTH, TARGET_HEIGHT, GDK_INTERP_BILINEAR);
        g_object_unref(pb);
        if (scaled) {
            pixbuf_to_binary_array(scaled);
            g_object_unref(scaled);
            gtk_widget_set_sensitive(process_button, TRUE);
            refresh_all_views();
        }
    }
}

typedef struct ThumbData { int frame_index; GtkWidget *dialog; } ThumbData;

static void on_thumb_clicked(GtkWidget *widget, gpointer user_data)
{
    ThumbData *td = (ThumbData*)user_data;
    if (!td) return;
    if (g_cap.isOpened()) {
        g_cap.set(cv::CAP_PROP_POS_FRAMES, td->frame_index);
        cv::Mat frame; if (g_cap.read(frame)) { g_frame_index = (int)g_cap.get(cv::CAP_PROP_POS_FRAMES); show_cv_frame(frame); }
    }
    if (td->dialog) gtk_widget_destroy(td->dialog);
    free(td);
}

static void open_tiled_selector_clicked(GtkWidget *widget, gpointer data)
{
    if (!g_cap.isOpened()) {
        GtkWidget *err = gtk_message_dialog_new(GTK_WINDOW(window), GTK_DIALOG_DESTROY_WITH_PARENT,
                                                GTK_MESSAGE_ERROR, GTK_BUTTONS_CLOSE, "请先导入视频");
        gtk_dialog_run(GTK_DIALOG(err)); gtk_widget_destroy(err); return;
    }

    // 获取总帧数与采样步长
    int total = (int)g_cap.get(cv::CAP_PROP_FRAME_COUNT);
    if (total <= 0) total = 3000; // 某些容器不返回帧数
    const int maxThumbs = 200; // 限制最大缩略图数，避免内存与等待时间过大
    int step = total / maxThumbs; if (step < 1) step = 1;

    GtkWidget *dialog = gtk_dialog_new_with_buttons("选择帧", GTK_WINDOW(window), GTK_DIALOG_MODAL,
                                                    "关闭", GTK_RESPONSE_CLOSE, NULL);
    GtkWidget *content = gtk_dialog_get_content_area(GTK_DIALOG(dialog));
    GtkWidget *scroll = gtk_scrolled_window_new(NULL, NULL);
    gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(scroll), GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
    gtk_box_pack_start(GTK_BOX(content), scroll, TRUE, TRUE, 0);

    GtkWidget *grid = gtk_grid_new();
    gtk_grid_set_row_spacing(GTK_GRID(grid), 6);
    gtk_grid_set_column_spacing(GTK_GRID(grid), 6);
    gtk_container_add(GTK_CONTAINER(scroll), grid);

    // 生成缩略图
    const int cols = 6; // 每行几列
    int r = 0, c = 0;
    double saved_pos = g_cap.get(cv::CAP_PROP_POS_FRAMES);
    for (int i = 0; i < total; i += step) {
        g_cap.set(cv::CAP_PROP_POS_FRAMES, i);
        cv::Mat frame; if (!g_cap.read(frame)) break;
        cv::Mat bgr = (frame.channels()==3) ? frame : (cv::Mat)cv::Mat();
        if (frame.channels()!=3) cv::cvtColor(frame, bgr, cv::COLOR_GRAY2BGR);
        cv::Mat rgb; cv::cvtColor(bgr, rgb, cv::COLOR_BGR2RGB);
        cv::Mat thumb; cv::resize(rgb, thumb, cv::Size(TARGET_WIDTH, TARGET_HEIGHT), 0, 0, cv::INTER_AREA);
        GdkPixbuf *pb = pixbuf_from_rgb_mat_copy(thumb);
        if (!pb) continue;
        GtkWidget *img = gtk_image_new_from_pixbuf(pb);
        g_object_unref(pb);
        GtkWidget *btn = gtk_button_new();
        gtk_container_add(GTK_CONTAINER(btn), img);
        ThumbData *td = (ThumbData*)malloc(sizeof(ThumbData));
        td->frame_index = i; td->dialog = dialog;
        g_signal_connect(btn, "clicked", G_CALLBACK(on_thumb_clicked), td);
        gtk_grid_attach(GTK_GRID(grid), btn, c, r, 1, 1);
        if (++c >= cols) { c = 0; ++r; }
    }
    // 还原播放位置
    g_cap.set(cv::CAP_PROP_POS_FRAMES, saved_pos);

    gtk_widget_show_all(dialog);
    g_signal_connect_swapped(dialog, "response", G_CALLBACK(gtk_widget_destroy), dialog);
}

static void open_video_clicked(GtkWidget *widget, gpointer data) {
    GtkWidget *dialog = gtk_file_chooser_dialog_new("选择视频文件", GTK_WINDOW(window), GTK_FILE_CHOOSER_ACTION_OPEN,
                                                   "_取消", GTK_RESPONSE_CANCEL, "_打开", GTK_RESPONSE_ACCEPT, NULL);
    if (gtk_dialog_run(GTK_DIALOG(dialog)) == GTK_RESPONSE_ACCEPT) {
        char *filename = gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(dialog));
        g_video_path = filename ? filename : "";
        g_free(filename);
        if (!g_video_path.empty()) {
            if (g_cap.isOpened()) g_cap.release();
            g_cap.open(g_video_path);
            g_frame_index = 0;
            if (g_cap.isOpened()) {
                cv::Mat frame; if (g_cap.read(frame)) { g_frame_index = 1; show_cv_frame(frame);} else {
                    GtkWidget *err = gtk_message_dialog_new(GTK_WINDOW(window), GTK_DIALOG_DESTROY_WITH_PARENT,
                                                            GTK_MESSAGE_ERROR, GTK_BUTTONS_CLOSE, "无法读取首帧");
                    gtk_dialog_run(GTK_DIALOG(err)); gtk_widget_destroy(err);
                }
            } else {
                GtkWidget *err = gtk_message_dialog_new(GTK_WINDOW(window), GTK_DIALOG_DESTROY_WITH_PARENT,
                                                        GTK_MESSAGE_ERROR, GTK_BUTTONS_CLOSE, "无法打开视频");
                gtk_dialog_run(GTK_DIALOG(err)); gtk_widget_destroy(err);
            }
        }
    }
    gtk_widget_destroy(dialog);
}

static void next_frame_clicked(GtkWidget *widget, gpointer data) {
    if (!g_cap.isOpened()) return;
    cv::Mat frame; if (g_cap.read(frame)) { ++g_frame_index; show_cv_frame(frame);} }

static void prev_frame_clicked(GtkWidget *widget, gpointer data) {
    if (!g_cap.isOpened()) return;
    double pos = g_cap.get(cv::CAP_PROP_POS_FRAMES);
    double back = pos - 2; if (back < 0) back = 0;
    g_cap.set(cv::CAP_PROP_POS_FRAMES, back);
    cv::Mat frame; if (g_cap.read(frame)) { g_frame_index = (int)g_cap.get(cv::CAP_PROP_POS_FRAMES); show_cv_frame(frame);} }
#endif
