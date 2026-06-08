#ifndef AMG8833_H
#define AMG8833_H

#define AMG8833_PIXEL_NUM 64

/* 64画素分の温度データ（単位：℃） */
typedef struct {
	float data[AMG8833_PIXEL_NUM];
} amg8833_pixels_t;

/* AMG8833のハンドル */
typedef struct {
	int fd;
} amg8833_handle_t;

/* AMG8833の集計データ */
typedef struct {
    amg8833_pixels_t pixels;
    float min_temp;
    float max_temp;
    float avg_temp;
} amg8833_data_t;

/* AMG8833を初期化する */
int amg8833_init(amg8833_handle_t *handle, const char *dev_path);

/* AMG8833の64画素分の温度データを読み出す */
int amg8833_read(amg8833_handle_t *handle, amg8833_pixels_t *pixels);

/* AMG8833の集計データを取得する */
int amg8833_get_data(amg8833_handle_t *handle, amg8833_data_t *data);

/*
 * ヒートマップ画像を生成し、保存先パスを返す。
 * 返される文字列は内部バッファを指すため、次回呼び出しで上書きされる。
 */
const char* amg8833_get_heatmap_path(amg8833_handle_t *handle);

/* AMG8833との通信を終了する */
void amg8833_close(amg8833_handle_t *handle);

#endif /* AMG8833_H */