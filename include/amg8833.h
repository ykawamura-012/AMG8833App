#ifndef AMG8833_H
#define AMG8833_H

#define AMG8833_PIXEL_NUM 64

/* 64画素分の温度データ（単位：℃） */
typedef struct {
	float data[AMG8833_PIXEL_NUM];
} amg8833_pixels_t;

/* AMG8833を初期化する */
int amg8833_init(const char *dev_path);

/* AMG8833の64画素分の温度データを読み出す */
init amg8833_read(amg8833_pixels_t *pixels);

/*
* ヒートマップ画像を生成し、保存先パスを返す。
* 返される文字列は内部バッファを指すため、次回呼び出しで上書きされる。
*/
const char* amg8833_get_heatmap_path(void);

/* AMG8833との通信を終了する */
void amg8833_close(void);

#endif /* AMG8833_H */