/*-----------------------Include---------------------*/
#include "amg8833.h"
#include "heatmap.h"
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <linux/i2c-dev.h>
#include <stdint.h>
#include <stdio.h>
#include <time.h>
/*------------------------Macro----------------------*/
#define AMG8833_ADDR               0x69
#define AMG8833_REG_PCLT           0x00
#define AMG8833_PCLT_NORMAL        0x00
#define AMG8833_REG_FPSC           0x02
#define AMG8833_FPSC_10_HZ         0x00
#define AMG8833_FPSC_1_HZ          0x01
#define AMG8833_REG_PIXEL_BASE     0x80
#define AMG8833_POWERUP_DELAY_US   50000

/* ヒートマップ画像出力用マクロ */
#define AMG8833_HEATMAP_DIR        "/tmp"
#define AMG8833_HEATMAP_PREFIX     "amg8833"
#define AMG8833_HEATMAP_MIN_TEMP   18.0f
#define AMG8833_HEATMAP_MAX_TEMP   40.0f
/*----------------------Variable---------------------*/
static const uint8_t g_addr = AMG8833_ADDR;
static char g_heatmap_path[256] = {0};
static unsigned int g_heatmap_seq = 0;
/******************************************************************************
*
* make_heatmap_path
*    ヒートマップ画像の保存先パスを生成する。
*
* 引数：
*   out      - 出力先バッファ
*   out_size - バッファサイズ
*
* 戻り値：
*   0  - 成功
*   -1 - 失敗
*/
static int make_heatmap_path(char *out, size_t out_size) {
	time_t now;
	struct tm tm_info;
	char timestamp[32];
	int n;
	
	if (!out || out_size == 0) {
		return -1;
	}
	
	/* 現在時刻を取得 */
	now = time(NULL);
	
	/* Unix時間を年月日時に変換 */
	if (localtime_r(&now, &tm_info) == NULL) {
		return -1;
	}
	
	/* タイムスタンプの文字列にフォーマット */
	if (strftime(timestamp, sizeof(timestamp), "%Y%m%d_%H%M%S", &tm_info) == 0) {
		return -1;
	}
	
	/* 保存先パス作成 */
	n = snprintf(out,
	             out_size,
	             "%s/%s_%s_%03u.png",
	             AMG8833_HEATMAP_DIR,
	             AMG8833_HEATMAP_PREFIX,
	             timestamp,
	             g_heatmap_seq++);
	/* オーバーフローチェック */
	if (n < 0 || (size_t)n >= out_size) {
		return -1;
	}
	
	return 0;
}

/******************************************************************************
*
* amg8833_init
*    I2Cデバイスをオープンし、AMG8833をNORMALモードに設定する。
*
* 引数：
*   handle   - AMG8833のハンドル
*   dev_path - I2Cデバイスファイルパス（例："/dev/i2c-1"）
*
* 戻り値：
*   0  - 成功
*   -1 - 失敗
*/
int amg8833_init(amg8833_handle_t *handle, const char *dev_path) {
	uint8_t fpsc[2] = {AMG8833_REG_FPSC, AMG8833_FPSC_10_HZ};
	uint8_t pclt[2] = {AMG8833_REG_PCLT, AMG8833_PCLT_NORMAL};
	
	/* 引数チェック */
	if (handle == NULL || dev_path == NULL) {
		fprintf(stderr, "amg8833_init: invalid arguments\n");
		return -1;
	}

	/* 既にオープンされている場合はクローズ */
	if (handle->fd >= 0) {
		close(handle->fd);
		handle->fd = -1;
	}
	
	/* I2Cデバイスファイルをオープン */
	handle->fd = open(dev_path, O_RDWR);
	if (handle->fd < 0) {
		perror("open");
		return -1;
	}
	
	/* I2Cスレーブアドレスを設定 */
	if (ioctl(handle->fd, I2C_SLAVE, g_addr) < 0) {
		perror("ioctl");
		close(handle->fd);
		handle->fd = -1;
		return -1;
	}
	
	/* AMG8833をNORMALモードに設定 */
	if (write(handle->fd, pclt, 2) != 2) {
		perror("amg8833: write reg");
		close(handle->fd);
		handle->fd = -1;
		return -1;
	}
	/* モード変更のため50ms待つ */
	usleep(AMG8833_POWERUP_DELAY_US);
	
	/* AMG8833のフレームレート設定（10Hz） */
	if (write(handle->fd, fpsc, 2) != 2) {
		perror("amg8833: write reg");
		close(handle->fd);
		handle->fd = -1;
		return -1;
	}
	
	return 0;
}

/******************************************************************************
*
* amg8833_read
*    AMG8833の64画素分の温度データを読み出す。
*    各画素データは12bit符号付き値であり、1LSB=0.25℃。
*
* 引数：
*   handle - AMG8833のハンドル
*   pixels - 温度データ格納先
*
* 戻り値：
*   0  - 成功
*   -1 - 失敗
*/
int amg8833_read(amg8833_handle_t *handle, amg8833_pixels_t *pixels) {
	uint8_t reg = AMG8833_REG_PIXEL_BASE;
	uint8_t buf[AMG8833_PIXEL_NUM * 2];
	int16_t raw;
	int i;
	
	/* 引数チェック */
	if (handle == NULL || pixels == NULL) {
		fprintf(stderr, "amg8833_read: invalid arguments\n");
		return -1;
	}

	/* デバイスがオープンされているかチェック */
	if (handle->fd < 0) {
		fprintf(stderr, "amg8833_read: device not initialized\n");
		return -1;
	}
	
	/* 読み出し開始レジスタを指定 */
	if (write(handle->fd, &reg, 1) != 1) {
		perror("amg8833: write reg"); 
		return -1;
	}
	
	/* 温度データを一括読み出し */
	if (read(handle->fd, buf, sizeof(buf)) != sizeof(buf)) {
		perror("amg8833: read reg");
		return -1;
	}
	
	/* 生データ → 温度(℃)に変換 */
	for (i = 0; i < AMG8833_PIXEL_NUM; i++) {
		raw = (int16_t)(buf[i * 2 + 1] << 8) | buf[i * 2];
		/* 12bit符号拡張 */
		if (raw & 0x8000) {
			raw |= 0xF000;
		}
		pixels->data[i] = raw * 0.25f;
	}
	
	return 0;
}
/******************************************************************************
 * 
 * amg8833_get_data
 *    AMG8833の64画素分の温度データを読み出し、集計データを計算して構造体に格納する。
 * 
 * 引数：
 *   handle - AMG8833のハンドル
 *   data   - 集計データ格納先
 * 
 * 戻り値：
 *  0  - 成功
 *  -1 - 失敗
 */
int amg8833_get_data(amg8833_handle_t *handle, amg8833_data_t *data)
{
    amg8833_pixels_t pixels;
    float sum = 0.0f;

    /* 引数チェック */
	if (handle == NULL || data == NULL) {
		fprintf(stderr, "amg8833_get_data: invalid arguments\n");
		return -1;
	}

    /* 温度データを読み出し */
	if (amg8833_read(handle, &pixels) != 0) {
        return -1;
    }

	 /* 最小値、最大値、平均値を計算し、構造体に格納 */
    data->min_temp = pixels.data[0];
    data->max_temp = pixels.data[0];

    for (int i = 0; i < AMG8833_PIXEL_NUM; i++) {
        data->pixels[i] = pixels.data[i];

        if (pixels.data[i] < data->min_temp) {
            data->min_temp = pixels.data[i];
        }

        if (pixels.data[i] > data->max_temp) {
            data->max_temp = pixels.data[i];
        }

        sum += pixels.data[i];
    }

    data->avg_temp = sum / AMG8833_PIXEL_NUM;

    return 0;
}

/******************************************************************************
*
* amg8833_get_heatmap_path
*    AMG8833から温度データを読み出し、ヒートマップPNGを新規保存して、
*    その保存先パスを返す。
*
* 引数：
*   handle - AMG8833のハンドル
*
* 戻り値：
*   成功時 : 保存したPNGファイルパス
*   失敗時 : NULL
*/
const char* amg8833_get_heatmap_path(amg8833_handle_t *handle) {
	amg8833_pixels_t pixels;
	
	/* 引数チェック */
	if (handle == NULL) {
		fprintf(stderr, "amg8833_get_heatmap_path: invalid arguments\n");
		return NULL;
	}
	
	/* センサ読み取り */
	if (amg8833_read(handle, &pixels) != 0) {
		fprintf(stderr, "amg8833_get_heatmap_path: read failed\n");
		return NULL;
	}
	
	/* 保存先パス作成 */
	if (make_heatmap_path(g_heatmap_path, sizeof(g_heatmap_path)) != 0) {
		fprintf(stderr, "amg8833_get_heatmap_path: path generation failed\n");
		return NULL;
	}
	
	/* PNG保存 */
	if (amg8833_save_heatmap_png(&pixels, 
	                             g_heatmap_path, 
	                             AMG8833_HEATMAP_MIN_TEMP, 
	                             AMG8833_HEATMAP_MAX_TEMP) != 0) {
		fprintf(stderr, "amg8833_get_heatmap_path: png save failed\n");
		g_heatmap_path[0] = '\0';
		return NULL;
	}
	
	return g_heatmap_path;
}

/******************************************************************************
*
* amg8833_close
*    I2Cデバイスをクローズする。
*
* 引数：
*   handle - AMG8833のハンドル
*/
void amg8833_close(amg8833_handle_t *handle) {
	/* 引数チェック */
	if (handle == NULL) {
		return;
	}

	/* ファイルディスクリプタをクローズ */
	if (handle->fd >= 0) {
		close(handle->fd);
		handle->fd = -1;
	}
	/* 内部状態の初期化 */
	g_heatmap_path[0] = '\0';
}