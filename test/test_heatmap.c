#include <stdio.h>
#include "amg8833.h"
#include "heatmap.h"

int main(void) {
	amg8833_pixels_t p;
	
	/* 8x8 に温度グラデーションを作る */
	for (int y = 0; y < 8; y++) {
		for (int x = 0; x < 8; x++) {
			p.data[y*8 + x] = 18.0f + (float)(y*8 + x) * (22.0f / 63.0f);
		}
	}
	
	if (amg8833_save_heatmap_png(&p, "heatmap_test.png", 18.0f, 40.0f) != 0) {
		fprintf(stderr, "failed\n");
		return 1;
	}
	printf("ok: heatmap_test.png\n");
	return 0;
}