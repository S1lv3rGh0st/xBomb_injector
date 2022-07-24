#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <fcntl.h>
#include "injector.h"

#include <sys/types.h>
#include <sys/stat.h>
#include <dirent.h>
#include <unistd.h>
#include <limits.h>

#define INVALID_PID -1
static pid_t find_process(const char *name)
{
    DIR *dir = opendir("/proc");
    struct dirent *dent;
    pid_t pid = -1;

    if (dir == NULL) {
        fprintf(stderr, "Failed to read proc file system.\n");
        exit(1);
    }
    while ((dent = readdir(dir)) != NULL) {
        char path[sizeof(dent->d_name) + 11];
        char exepath[PATH_MAX];
        ssize_t len;
        char *exe;

        if (dent->d_name[0] < '1' || '9' < dent->d_name[0]) {
            continue;
        }
        sprintf(path, "/proc/%s/exe", dent->d_name);
        len = readlink(path, exepath, sizeof(exepath) - 1);
        if (len == -1) {
            continue;
        }
        exepath[len] = '\0';
        exe = strrchr(exepath, '/');
        if (exe != NULL && strcmp(exe + 1, name) == 0) {
            pid = atoi(dent->d_name);
            break;
        }
    }
    closedir(dir);
    return pid;
}


void usage(char* prog_name) {
	printf("Usage: %s library\n", prog_name);
}


int main(int argc, char** argv) {
	if (argc < 2) {
		usage(argv[0]);
		exit(0);
	}

	injector_pid_t pid = INVALID_PID;
    injector_t *injector;
    int opt;
    int i;
    char *endptr;

	char* lib = argv[1];
    pid = find_process("xbomb\x00");
    if (pid == INVALID_PID) {
        fprintf(stderr, "could not find the process: xbomb\n");
        return 1;
    }
    printf("targeting process with pid %d\n", pid);
    umask(000);
    if (!mkfifo("/tmp/xbomb_dumped", 0666)) {
    	if (errno == EEXIST) {
    		printf("Pipe exist, continue..\n");
    	}
    }
    printf("Attaching to target process...\n");
    if (injector_attach(&injector, pid) != 0) {
        printf("%s\n", injector_error());
        return 1;
    }
    printf("Forking...\n");
    int local_pid = fork();
    if (local_pid) {
    	sleep(1);
	    void* handle;
	    if (injector_inject(injector, lib, &handle) == 0) {
	        // printf("\"%s\" successfully injected\n", lib);
	    } else {
	        fprintf(stderr, "could not inject \"%s\"\n", lib);
	        fprintf(stderr, "  %s\n", injector_error());
	    }
	    sleep(2);
	    if (injector_uninject(injector, handle) != 0) {
        	printf("UNINJECT ERROR: %s\n", injector_error());
    	}
	    injector_detach(injector);
    } else {
    	printf("Opening pipe\n");
    	int file_h = open("/tmp/xbomb_dumped", O_RDONLY | O_ASYNC);
		printf("Received game map:\n");
	    char buf[1024] = {0};
	    while (read(file_h, buf, 1024) > 0) {
		    printf("%s", buf);
		    memset(buf, 0, sizeof(buf));
		}
		printf("End of data\n");
	    close(file_h);
	    remove("/tmp/xbomb_dumped");
	}
            

}