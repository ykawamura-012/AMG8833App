/*-----------------------Define----------------------*/
#define STB_IMAGE_WRITE_IMPLEMENTATION
/*-----------------------Include---------------------*/
#include "stb_image_write.h"
#include "heatmap.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
/*------------------------Macro----------------------*/
#define SRC_SIZE          8     /* 取得データサイズ(8x8) */
#define DST_SIZE          128   /* 拡大データサイズ */
#define COLORBAR_WIDTH    8     /* カラーバーの幅 */
#define COLORBAR_MARGIN   20    /* カラーバーの上下マージン */
#define FONT_SCALE        1     /* フォントの拡大倍率 */
#define FONT_WIDTH        6     /* フォント1文字の幅 */
/*----------------------Variable---------------------*/
/* Turboカラーマップ 256段LUT */
static const unsigned char turbo_colormap[256][3] = {
	{48,18,59},{50,21,67},{51,24,74},{52,27,81},{53,30,88},{54,33,95},{55,36,102},{56,39,109},
	{57,42,115},{58,45,121},{59,47,128},{60,50,134},{61,53,139},{62,56,145},{63,59,151},{63,62,156},
	{64,64,162},{65,67,167},{65,70,172},{66,73,177},{66,75,181},{67,78,186},{68,81,191},{68,84,195},
	{68,86,199},{69,89,203},{69,92,207},{69,94,211},{70,97,214},{70,100,218},{70,102,221},{70,105,224},
	{70,107,227},{71,110,230},{71,113,233},{71,115,235},{71,118,238},{71,120,240},{71,123,242},{70,125,244},
	{70,128,246},{70,130,248},{70,133,250},{70,135,251},{69,138,252},{69,140,253},{68,143,254},{67,145,254},
	{66,148,255},{65,150,255},{64,153,255},{62,155,254},{61,158,254},{59,160,253},{58,163,252},{56,165,251},
	{55,168,250},{53,171,248},{51,173,247},{49,175,245},{47,178,244},{46,180,242},{44,183,240},{42,185,238},
	{40,188,235},{39,190,233},{37,192,231},{35,195,228},{34,197,226},{32,199,223},{31,201,221},{30,203,218},
	{28,205,216},{27,208,213},{26,210,210},{26,212,208},{25,213,205},{24,215,202},{24,217,200},{24,219,197},
	{24,221,194},{24,222,192},{24,224,189},{25,226,187},{25,227,185},{26,228,182},{28,230,180},{29,231,178},
	{31,233,175},{32,234,172},{34,235,170},{37,236,167},{39,238,164},{42,239,161},{44,240,158},{47,241,155},
	{50,242,152},{53,243,148},{56,244,145},{60,245,142},{63,246,138},{67,247,135},{70,248,132},{74,248,128},
	{78,249,125},{82,250,122},{85,250,118},{89,251,115},{93,252,111},{97,252,108},{101,253,105},{105,253,102},
	{109,254,98},{113,254,95},{117,254,92},{121,254,89},{125,255,86},{128,255,83},{132,255,81},{136,255,78},
	{139,255,75},{143,255,73},{146,255,71},{150,254,68},{153,254,66},{156,254,64},{159,253,63},{161,253,61},
	{164,252,60},{167,252,58},{169,251,57},{172,251,56},{175,250,55},{177,249,54},{180,248,54},{183,247,53},
	{185,246,53},{188,245,52},{190,244,52},{193,243,52},{195,241,52},{198,240,52},{200,239,52},{203,237,52},
	{205,236,52},{208,234,52},{210,233,53},{212,231,53},{215,229,53},{217,228,54},{219,226,54},{221,224,55},
	{223,223,55},{225,221,55},{227,219,56},{229,217,56},{231,215,57},{233,213,57},{235,211,57},{236,209,58},
	{238,207,58},{239,205,58},{241,203,58},{242,201,58},{244,199,58},{245,197,58},{246,195,58},{247,193,58},
	{248,190,57},{249,188,57},{250,186,57},{251,184,56},{251,182,55},{252,179,54},{252,177,54},{253,174,53},
	{253,172,52},{254,169,51},{254,167,50},{254,164,49},{254,161,48},{254,158,47},{254,155,45},{254,153,44},
	{254,150,43},{254,147,42},{254,144,41},{253,141,39},{253,138,38},{252,135,37},{252,132,35},{251,129,34},
	{251,126,33},{250,123,31},{249,120,30},{249,117,29},{248,114,28},{247,111,26},{246,108,25},{245,105,24},
	{244,102,23},{243,99,21},{242,96,20},{241,93,19},{240,91,18},{239,88,17},{237,85,16},{236,83,15},
	{235,80,14},{234,78,13},{232,75,12},{231,73,12},{229,71,11},{228,69,10},{226,67,10},{225,65,9},
	{223,63,8},{221,61,8},{220,59,7},{218,57,7},{216,55,6},{214,53,6},{212,51,5},{210,49,5},{208,47,5},
	{206,45,4},{204,43,4},{202,42,4},{200,40,3},{197,38,3},{195,37,3},{193,35,2},{190,33,2},{188,32,2},
	{185,30,2},{183,29,2},{180,27,1},{178,26,1},{175,24,1},{172,23,1},{169,22,1},{167,20,1},{164,19,1},
	{161,18,1},{158,16,1},{155,15,1},{152,14,1},{149,13,1},{146,11,1},{142,10,1},{139,9,2},{136,8,2},
	{133,7,2},{129,6,2},{126,5,2},{122,4,3}
};
/******************************************************************************
*
* temp_to_color
*    正規化した温度値をRGB値へ変換する。
*
* 引数：
*   norm - 0.0～1.0 に正規化した温度値
*   r    - 赤成分の出力先
*   g    - 緑成分の出力先
*   b    - 青成分の出力先
*
* 戻り値：なし
*/
static void temp_to_color(float norm,
                          unsigned char *r,
                          unsigned char *g,
                          unsigned char *b)
{
	int idx;
	
	/* 範囲外のアクセスを防ぐ */
	if (norm < 0) norm = 0.0f;
	if (norm > 1) norm = 1.0f;
	
	/* 0～255の整数に変換 */
	idx = (int)(norm * 255.0f + 0.5f);
	
	/* Turboカラーマッピング */
	*r = turbo_colormap[idx][0];
	*g = turbo_colormap[idx][1];
	*b = turbo_colormap[idx][2];
}

/******************************************************************************
 * 
 * set_pixel
 *   画像バッファの指定座標にRGB値をセットする。
 *  座標が画像範囲外の場合は何もしない。
 * 
 * 引数：
 *   image - 画像バッファ（幅×高さ×3のサイズでRGB値が格納されている）
 *   x     - X座標
 *   y     - Y座標
 *   r     - 赤成分
 *   g     - 緑成分
 * 	 b     - 青成分
 * 
 * 戻り値：なし
 */
static void set_pixel(unsigned char *image,
                      int x,
                      int y,
                      unsigned char r,
                      unsigned char g,
                      unsigned char b)
{
	int idx;

	if (x < 0 || x >= DST_SIZE) return;
	if (y < 0 || y >= DST_SIZE) return;

	idx = (y * DST_SIZE + x) * 3;
	image[idx + 0] = r;
	image[idx + 1] = g;
	image[idx + 2] = b;
}

/******************************************************************************
 *
 * draw_cross
 *  画像バッファの指定座標に、中心が(cx, cy)の十字マークを描画する。
 * 
 * 引数：
 *  image - 画像バッファ（幅×高さ×3のサイズでRGB値が格納されている）
 *  cx    - 十字マークの中心X座標
 *  cy    - 十字マークの中心Y座標
 * 
 * 戻り値：なし
 */
static void draw_cross(unsigned char *image, int cx, int cy)
{
	int i;

	/* 十字マークは、中心から上下左右に4ピクセルずつ伸びる形にする */
	for (i = -4; i <= 4; i++) {
		set_pixel(image, cx + i, cy, 255, 255, 255);
		set_pixel(image, cx, cy + i, 255, 255, 255);
	}
}

/******************************************************************************
 *
 * get_font5x7
 *  5x7ドットのビットマップフォントデータを取得する。
 * 
 * 引数：
 *  c - 文字（'0'～'9', '.', '-', 'C', ' '）
 * 
 * 戻り値：
 *  文字に対応する7バイトのビットマップデータへのポインタ。
 *  対応する文字がない場合はスペースのビットマップを返す。
 */
static const unsigned char *get_font5x7(char c)
{
	static const unsigned char font_0[7] = {0x0E,0x11,0x13,0x15,0x19,0x11,0x0E};
	static const unsigned char font_1[7] = {0x04,0x0C,0x04,0x04,0x04,0x04,0x0E};
	static const unsigned char font_2[7] = {0x0E,0x11,0x01,0x02,0x04,0x08,0x1F};
	static const unsigned char font_3[7] = {0x1E,0x01,0x01,0x0E,0x01,0x01,0x1E};
	static const unsigned char font_4[7] = {0x02,0x06,0x0A,0x12,0x1F,0x02,0x02};
	static const unsigned char font_5[7] = {0x1F,0x10,0x10,0x1E,0x01,0x01,0x1E};
	static const unsigned char font_6[7] = {0x0E,0x10,0x10,0x1E,0x11,0x11,0x0E};
	static const unsigned char font_7[7] = {0x1F,0x01,0x02,0x04,0x08,0x08,0x08};
	static const unsigned char font_8[7] = {0x0E,0x11,0x11,0x0E,0x11,0x11,0x0E};
	static const unsigned char font_9[7] = {0x0E,0x11,0x11,0x0F,0x01,0x01,0x0E};
	static const unsigned char font_dot[7] = {0x00,0x00,0x00,0x00,0x00,0x0C,0x0C};
	static const unsigned char font_minus[7] = {0x00,0x00,0x00,0x1F,0x00,0x00,0x00};
	static const unsigned char font_c[7] = {0x0E,0x11,0x10,0x10,0x10,0x11,0x0E};
	static const unsigned char font_space[7] = {0x00,0x00,0x00,0x00,0x00,0x00,0x00};

	switch (c) {
	case '0': return font_0;
	case '1': return font_1;
	case '2': return font_2;
	case '3': return font_3;
	case '4': return font_4;
	case '5': return font_5;
	case '6': return font_6;
	case '7': return font_7;
	case '8': return font_8;
	case '9': return font_9;
	case '.': return font_dot;
	case '-': return font_minus;
	case 'C': return font_c;
	case ' ': return font_space;
	default:  return font_space;
	}
}

/******************************************************************************
 *
 * draw_char
 *  画像バッファの指定座標に、指定した文字を描画する。
 *  フォントは5x7ドットで、必要に応じて拡大して描画する。
 * 
 * 引数：
 *  image - 画像バッファ（幅×高さ×3のサイズでRGB値が格納されている）
 *  x     - 描画開始X座標
 *  y     - 描画開始Y座標
 *  c     - 描画する文字
 *  r     - 赤成分
 *  g     - 緑成分
 *  b     - 青成分
 * 
 * 戻り値：なし
 */
static void draw_char(unsigned char *image,
                      int x,
                      int y,
                      char c,
                      unsigned char r,
                      unsigned char g,
                      unsigned char b)
{
	const unsigned char *font;
	int row, col;
	int sx, sy;

	font = get_font5x7(c);

	for (row = 0; row < 7; row++) {
		for (col = 0; col < 5; col++) {
			if (font[row] & (1 << (4 - col))) {
				for (sy = 0; sy < FONT_SCALE; sy++) {
					for (sx = 0; sx < FONT_SCALE; sx++) {
						set_pixel(image,
						          x + col * FONT_SCALE + sx,
						          y + row * FONT_SCALE + sy,
						          r, g, b);
					}
				}
			}
		}
	}
}

/******************************************************************************
 *
 * draw_text
 *  画像バッファの指定座標に、指定した文字列を描画する。
 *  フォントは5x7ドットで、必要に応じて拡大して描画する。
 * 
 * 引数：
 *  image - 画像バッファ（幅×高さ×3のサイズでRGB値が格納されている）
 *  x     - 描画開始X座標
 *  y     - 描画開始Y座標
 *  text  - 描画する文字列（NULL終端）
 *  r     - 赤成分
 *  g     - 緑成分
 *  b     - 青成分
 * 
 * 戻り値：なし
 */
static void draw_text(unsigned char *image,
                      int x,
                      int y,
                      const char *text,
                      unsigned char r,
                      unsigned char g,
                      unsigned char b)
{
	int i;
	int cx;

	cx = x;

	for (i = 0; text[i] != '\0'; i++) {
		draw_char(image, cx, y, text[i], r, g, b);
		cx += 6 * FONT_SCALE;
	}
}

/******************************************************************************
 *
 * draw_text_with_shadow
 *  文字列を白色で描画し、その下に黒い影を描くことで、明るい背景でも文字が見やすくなるようにする。
 * 
 * 引数：
 *  image - 画像バッファ（幅×高さ×3のサイズでRGB値が格納されている）
 *  x     - 描画開始X座標
 *  y     - 描画開始Y座標
 *  text  - 描画する文字列（NULL終端）
 * 
 * 戻り値：なし
 */
static void draw_text_with_shadow(unsigned char *image,
                                  int x,
                                  int y,
                                  const char *text)
{
	/* 黒の影を先に描く */
	draw_text(image, x + 1, y + 1, text, 0, 0, 0);

	/* 白文字を上から描く */
	draw_text(image, x, y, text, 255, 255, 255);
}

/******************************************************************************
 *
 * draw_colorbar
 *  画像の右端に、温度のカラーバーを描画する。
 *  カラーバーは、上が高温（赤）、下が低温（青）になるようにする。
 * 
 * 引数：
 *  image - 画像バッファ（幅×高さ×3のサイズでRGB値が格納されている）
 * 
 * 戻り値：なし
 */
static void draw_colorbar(unsigned char *image)
{
    int x, y, idx;
    int bar_top;
    int bar_bottom;
    int bar_height;
    unsigned char r, g, b;
    float norm;

    bar_top = COLORBAR_MARGIN;
    bar_bottom = DST_SIZE - COLORBAR_MARGIN;
    bar_height = bar_bottom - bar_top;

    for (y = bar_top; y < bar_bottom; y++) {
        norm = 1.0f -
               (float)(y - bar_top) / (float)(bar_height - 1);

        temp_to_color(norm, &r, &g, &b);

        for (x = DST_SIZE - COLORBAR_WIDTH; x < DST_SIZE; x++) {
            idx = (y * DST_SIZE + x) * 3;
            image[idx + 0] = r;
            image[idx + 1] = g;
            image[idx + 2] = b;
        }
    }

    /* カラーバー左端の黒線 */
    for (y = bar_top; y < bar_bottom; y++) {
        set_pixel(image, DST_SIZE - COLORBAR_WIDTH - 1, y, 0, 0, 0);
    }
}

/******************************************************************************
*
* bilinear_sample
*    8x8 の温度データから、指定座標の温度値をバイリニア補間で求める。
*
* 引数：
*   pixels - AMG8833の温度データ
*   src_x  - 元画像上のX座標
*   src_y  - 元画像上のY座標
*
* 戻り値：
*   補間後の温度値
*/
static float bilinear_sample(const amg8833_pixels_t *pixels, float src_x, float src_y)
{
	int x0, y0, x1, y1;
	float dx, dy;
	float v00, v01, v10, v11;
	float v0, v1;
	
	/* 座標の範囲外アクセスを防ぐ */
	if (src_x < 0.0f) src_x = 0.0f;
	if (src_x > (float)(SRC_SIZE - 1)) src_x = (float)(SRC_SIZE - 1);
	if (src_y < 0.0f) src_y = 0.0f;
	if (src_y > (float)(SRC_SIZE - 1)) src_y = (float)(SRC_SIZE - 1);
	
	/* 周囲4点の座標を取得 */
	x0 = (int)src_x;
	y0 = (int)src_y;
	x1 = (x0 < SRC_SIZE - 1) ? (x0 + 1) : x0;
	y1 = (y0 < SRC_SIZE - 1) ? (y0 + 1) : y0;
	
	/* 距離を計算 */
	dx = src_x - (float)x0;
	dy = src_y - (float)y0;
	
	/* 4点の温度値を取得 */
	v00 = pixels->data[y0 * SRC_SIZE + x0];
	v01 = pixels->data[y0 * SRC_SIZE + x1];
	v10 = pixels->data[y1 * SRC_SIZE + x0];
	v11 = pixels->data[y1 * SRC_SIZE + x1];
	
	/* 重み付け */
	v0 = v00 * (1.0f - dx) + v01 * dx;
	v1 = v10 * (1.0f - dx) + v11 * dx;
	
	return v0 * (1.0f - dy) + v1 * dy;
}

/******************************************************************************
 * 
 * amg8833_calc_heatmap_range
 *    AMG8833の64画素の温度データから、ヒートマップの表示に適した温度範囲を自動計算する。
 *	  温度差が小さい場合は、最低限の幅を持たせる。
 *
 * 引数：
 *   pixels          - AMG8833の温度データ
 *   min_temp         - 計算された最小温度の出力先
 *   max_temp         - 計算された最大温度の出力先
 *   min_range_width  - 最小温度範囲幅（例：5.0f）。これより温度差が小さい場合は、中心を保ったまま幅を広げる。
 *
 * 戻り値：
 *   0  - 成功
 *   -1 - 失敗（引数エラーなど）
 */
int amg8833_calc_heatmap_range(const amg8833_pixels_t *pixels,
                               float *min_temp,
                               float *max_temp,
                               float min_range_width)
{
	int i;
	float center;
	float range;

	/* 引数チェック */
	if (pixels == NULL || min_temp == NULL || max_temp == NULL || min_range_width < 0.0f) {
		fprintf(stderr, "amg8833_calc_heatmap_range: invalid arguments\n");
		return -1;
	}

	/* 最小値と最大値を求める */
	*min_temp = pixels->data[0];
	*max_temp = pixels->data[0];

	for (i = 1; i < AMG8833_PIXEL_NUM; i++) {
		if (pixels->data[i] < *min_temp) {
			*min_temp = pixels->data[i];
		}

		if (pixels->data[i] > *max_temp) {
			*max_temp = pixels->data[i];
		}
	}

	/*
	 * 温度差が小さすぎる場合は、最低限の幅を持たせる。
	 * 例：25.0～25.5℃しかない場合でも、5℃幅などに広げる。
	 */
	range = *max_temp - *min_temp;

	if (min_range_width > 0.0f && range < min_range_width) {
		center = (*max_temp + *min_temp) / 2.0f;
		*min_temp = center - (min_range_width / 2.0f);
		*max_temp = center + (min_range_width / 2.0f);
	}

	return 0;
}

/******************************************************************************
*
* amg8833_save_heatmap_png
*    AMG8833のピクセルデータを拡大補間し、PNG画像として出力する。
*
* 引数：
*   pixels   - AMG8833の温度データ
*   filename - 保存先ファイルパス
*   min_temp - カラーマップ下限温度（人体検知デモ向け推奨設定：18.0f）
*   max_temp - カラーマップ上限温度（人体検知デモ向け推奨設定：40.0f）
*
* 戻り値：
*   0  - 成功
*   -1 - 失敗
*/
int amg8833_save_heatmap_png(const amg8833_pixels_t *pixels,
                             const char *filename,
                             float min_temp,
                             float max_temp)
{
	unsigned char *image;
	unsigned char r, g, b;
	int x, y, idx;
	int result;
	float temp, norm;
	float src_x, src_y;
	
	char max_text[16];
	char min_text[16];
	char hottest_text[16];

	int text_x;
	int text_y;
	
	int max_text_width;
	int min_text_width;
	int max_text_x;
	int min_text_x;
	
	int hottest_x;
	int hottest_y;
	float hottest_temp;
	int hottest_initialized;
	
	/* 引数チェック */
	if (pixels == NULL || filename == NULL) {
		fprintf(stderr, "amg8833_save_heatmap_png: invalid arguments\n");
		return -1;
	}
	
	/* ゼロ除算防止 */
	if (max_temp <= min_temp) {
		fprintf(stderr, "amg8833_save_heatmap_png: invalid temperature range\n");
		return -1;
	}
	
	/* 最高温度の位置を記録する変数を初期化 */
	hottest_x = 0;
	hottest_y = 0;
	hottest_temp = 0.0f;
	hottest_initialized = 0;

	/* 出力画像のバッファを確保（幅×高さ×3のサイズでRGB値を格納） */
	image = (unsigned char *)malloc(DST_SIZE * DST_SIZE * 3);
	if (!image) {
		fprintf(stderr, "amg8833_save_heatmap_png: failed to allocate image buffer\n");
		return -1;
	}
	
	/* DST_SIZE x DST_SIZE の出力画像の各ピクセルについてループ */
	for (y = 0; y < DST_SIZE; y++) {
		for (x = 0; x < DST_SIZE; x++) {
			
			/* 出力画像を入力8x8座標系へ変換 */
			src_x = (float)x * (float)(SRC_SIZE - 1) / (float)(DST_SIZE - 1);
			src_y = (float)y * (float)(SRC_SIZE - 1) / (float)(DST_SIZE - 1);
			
			/* 変換座標の温度をバイリニア補間で求める */
			temp = bilinear_sample(pixels, src_x, src_y);

			if (!hottest_initialized || temp > hottest_temp) {
				hottest_temp = temp;
				hottest_x = x;
				hottest_y = y;
				hottest_initialized = 1;
			}

			/* 温度を正規化 */
			norm = (temp - min_temp) / (max_temp - min_temp);
			
			/* 温度をカラーに変換 */
			temp_to_color(norm, &r, &g, &b);
			
			/* 配列にRGB値を格納 */
			idx = (y * DST_SIZE + x) * 3;
			image[idx + 0] = r;
			image[idx + 1] = g;
			image[idx + 2] = b;
		}
	}

	/* 最高温度の位置に十字マークを描画 */
	draw_cross(image, hottest_x, hottest_y);

	/* 最高温度のテキストを描画 */
	text_x = hottest_x + 8;
	text_y = hottest_y - 8;

	if (text_x > DST_SIZE - 40) {
		text_x = hottest_x - 40;
	}

	if (text_y < 8) {
		text_y = hottest_y + 8;
	}

	snprintf(hottest_text, sizeof(hottest_text), "%.1fC", hottest_temp);
	draw_text_with_shadow(image, text_x, text_y, hottest_text);
	
	/* カラーバーと温度テキストを描画 */
	draw_colorbar(image);
	
	snprintf(max_text, sizeof(max_text), "%.1fC", max_temp);
	snprintf(min_text, sizeof(min_text), "%.1fC", min_temp);
	
	/* 文字列の幅から、右端基準でx座標を決める */
	max_text_width = (int)strlen(max_text) * FONT_WIDTH * FONT_SCALE;
	min_text_width = (int)strlen(min_text) * FONT_WIDTH * FONT_SCALE;
	
	max_text_x = DST_SIZE - max_text_width - 2;
	min_text_x = DST_SIZE - min_text_width - 2;
	
	draw_text_with_shadow(image, max_text_x, COLORBAR_MARGIN - 10, max_text);
	draw_text_with_shadow(image, min_text_x, DST_SIZE - COLORBAR_MARGIN + 3, min_text);
	
	/* 配列をPNG画像で出力 */
	result = stbi_write_png(filename,
	                        DST_SIZE,
	                        DST_SIZE,
	                        3,
	                        image,
	                        DST_SIZE * 3);
	
	free(image);
	
	return (result != 0) ? 0 : -1;
}