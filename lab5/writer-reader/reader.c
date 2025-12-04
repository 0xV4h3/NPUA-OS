#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include "shm.h"

int main(int argc, char *argv[])
{
	(void)argc; (void)argv;
	int semid, transfers, bytes;
	struct shmseg *shmp;
	const char *shmfile = "/tmp/mmap_shmfile";
	int fdshm;

	/* Get IDs for semaphore set created by writer */
	semid = semget(SEM_KEY, 0, 0);

	if (semid == -1) {
		fprintf(stderr, "semget error");
		exit(EXIT_FAILURE);
	}

	fdshm = open(shmfile, O_RDONLY);
	if (fdshm == -1) {
		perror("open shmfile");
		exit(EXIT_FAILURE);
	}

	shmp = mmap(NULL, sizeof(struct shmseg), PROT_READ, MAP_SHARED, fdshm, 0);
	if (shmp == MAP_FAILED) {
		perror("mmap shmfile");
		close(fdshm);
		exit(EXIT_FAILURE);
	}

	/* Transfer blocks of data from shared memory to stdout */
	for (transfers = 0, bytes = 0; ; transfers++) {
		if (reserveSem(semid, READ_SEM) == -1) { /* Wait for our turn */
			fprintf(stderr, "reserveSem error");
			exit(EXIT_FAILURE);
		}

		if (shmp->cnt == 0) { /* Writer encountered EOF */
			break;
		}

		bytes += shmp->cnt;

		if (write(STDOUT_FILENO, shmp->buf, shmp->cnt) != shmp->cnt) {
			fprintf(stderr, "partial/failed write");
			exit(EXIT_FAILURE);
		}

		if (releaseSem(semid, WRITE_SEM) == -1) { /* Give writer a turn */
			fprintf(stderr, "releaseSem error");
			exit(EXIT_FAILURE);
		}
	}

	if (munmap(shmp, sizeof(struct shmseg)) == -1) {
		fprintf(stderr, "munmap error");
	}
	close(fdshm);

	/* Give writer one more turn, so it can clean up */
	if (releaseSem(semid, WRITE_SEM) == -1) {
		fprintf(stderr, "releaseSem error");
		exit(EXIT_FAILURE);
	}

	fprintf(stderr, "Received %d bytes (%d transfers)\n", bytes, transfers);
	exit(EXIT_SUCCESS);
}
