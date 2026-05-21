#include "amg8833.h"
#include <stdio.h>
#include <unistd.h>

int main(void)
{
	const char *path;
	
	printf("AMG8833 test start\n");
	
	if (amg8833_init("/dev/i2c-3") != 0) {
		fprintf(stderr, "ERROR: amg8833_init failed\n");
		return 1;
	}
	
	path = amg8833_get_heatmap_path();
	if (path == NULL) {
		fprintf(stderr, "ERROR: amg8833_get_heatmap_path failed\n");
		return 1;
	}
	
	printf("heatmap saved: %s\n", path);
	
	if (access(path, F_OK) != 0) {
		fprintf(stderr, "ERROR: output file dose not exist: %s\n", path);
		amg8833_close();
		return 1;
	}
	
	printf("output file exists\n");
	
	amg8833_close();
	
	printf("AMG8833 test finished successfully\n");
	return 0;
}