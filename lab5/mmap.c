#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <signal.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <sys/stat.h>

#define MEM_SIZE 15

static void usage(const char *prog) {
	fprintf(stderr, "Usage: %s file [new-value] [--sleep] [--sigbus] [--private] [--munmap]\n", prog);
	exit(EXIT_FAILURE);
}

void sigbus_handler(int sig) {
	fprintf(stderr, "Received SIGBUS (%d)\n", sig);
	_exit(EXIT_FAILURE);
}

int main(int argc, char *argv[])
{
	char *addr;
	int fd;
	int do_sleep = 0, do_sigbus = 0, do_private = 0, do_munmap = 0;

	if (argc < 2) usage(argv[0]);

	/* Parse simple flags at the end */
	for (int i = 1; i < argc; ++i) {
		if (strcmp(argv[i], "--sleep") == 0) do_sleep = 1;
		if (strcmp(argv[i], "--sigbus") == 0) do_sigbus = 1;
		if (strcmp(argv[i], "--private") == 0) do_private = 1;
		if (strcmp(argv[i], "--munmap") == 0) do_munmap = 1;
	}

	fd = open(argv[1], O_RDWR);

	if (fd == -1) {
		perror("open");
		exit(EXIT_FAILURE);
	}

	int flags = do_private ? MAP_PRIVATE : MAP_SHARED;

	addr = mmap(NULL, MEM_SIZE, PROT_READ | PROT_WRITE, flags, fd, 0);

	if (addr == MAP_FAILED) {
		perror("mmap");
		close(fd);
		exit(EXIT_FAILURE);
	}

	/* Keep fd open while we may ftruncate it to trigger SIGBUS */

	/* Output current contents */
	printf("Current string=%.*s\n", MEM_SIZE, addr);

	if (do_sleep) {
		/* Pause so you can inspect /proc/<pid>/maps; press Ctrl+Z while running */
		sleep(5);
	}

	if (do_sigbus) {
		/* Register handler */
		if (signal(SIGBUS, sigbus_handler) == SIG_ERR) {
			perror("signal");
			/* non-fatal */
		}

		/* Shrink the file to provoke SIGBUS on access */
		if (ftruncate(fd, 1) == -1) {
			perror("ftruncate");
		} else {
			/* Attempt to write past the truncated end */
			fprintf(stderr, "Attempting write to provoke SIGBUS...\n");
			addr[2] = 'X'; /* may raise SIGBUS */
		}
	}

	if (argc > 1 && argv[2] != NULL && argv[2][0] != '-') {
		/* argv[2] may be new-value unless it's a flag */
		if (strcmp(argv[2], "--sleep") != 0 && strcmp(argv[2], "--sigbus") != 0 &&
			strcmp(argv[2], "--private") != 0 && strcmp(argv[2], "--munmap") != 0) {

			if (strlen(argv[2]) >= MEM_SIZE) {
				fprintf(stderr, "'new-value' too large\n");
			} else {
				memset(addr, 0, MEM_SIZE);
				strncpy(addr, argv[2], MEM_SIZE - 1);
				if (msync(addr, MEM_SIZE, MS_SYNC) == -1) {
					perror("msync");
				}
				printf("Copied \"%s\" to mapped memory\n", argv[2]);
			}
		}
	}

	if (do_munmap) {
		if (munmap(addr, MEM_SIZE) == -1) {
			perror("munmap");
		} else {
			/* Access after munmap is undefined — likely SIGSEGV */
			fprintf(stderr, "Accessing unmapped memory (expected SIGSEGV/UB)...\n");
			volatile char c = addr[0];
			(void)c;
		}
	}

	if (close(fd) == -1) {
		perror("close");
	}

	return EXIT_SUCCESS;
}
