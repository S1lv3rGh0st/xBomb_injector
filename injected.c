#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>



extern void* menuButtonWidgetClass;
size_t menu_offset = 0x000000000209a80;

void parse_map(int width, int height, char* map, int file_h) {
	for (int y=0; y<height; y++) {
		for (int x=0; x<width; x++) {
			if (map[x*30+y] & 0x10) {
				write(file_h, "* ", 2);
			} else if (map[x*30+y] & 0x40) {
				write(file_h, ". ", 2);
			} else {
				char res[2] = {'0'+map[x*30+y], ' '};
				write(file_h, res, 2);
			}
		}
		write(file_h, "\n", 1);
	}
}

__attribute__((constructor))
void init()
{
	// *(int*)0 = (int)0;
	size_t map_off = 0x0000000000209AE0;
	size_t height_off = 0x000000000020A750;
	size_t width_off = 0x000000000020A754;
	void* exe_base = ((void*)&menuButtonWidgetClass)-menu_offset;
	char path_f[] = "/tmp/xbomb_dumped";
	int file_h = open(path_f, O_CREAT | O_WRONLY);
	// write(file_h, "CURRENT GAME	TABLE\n", strlen("HELLO WORLD\n"));

	int height = *(int*)(exe_base+height_off);
	int width = *(int*)(exe_base+width_off);
	
	char* map = (char*)(exe_base+map_off);

    parse_map(width, height, map, file_h);
    close(file_h);
}
