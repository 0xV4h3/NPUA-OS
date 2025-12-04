#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include "semun.h"
#include "shm.h"

int main(int argc, char *argv[])
{
	(void)argc; (void)argv;
	int semid, bytes, transfers;
	struct shmseg *shmp;
	union semun dummy;
	const char *shmfile = "/tmp/mmap_shmfile";
	int fdshm;

	semid = semget(SEM_KEY, 2, IPC_CREAT | OBJ_PERMS);

	if (semid == -1) {
		fprintf(stderr, "semget error");
		exit(EXIT_FAILURE);
	}

	if (initSemAvailable(semid, WRITE_SEM) == -1) {
		fprintf(stderr, "initSemAvailable error");
		exit(EXIT_FAILURE);
	}

	if (initSemInUse(semid, READ_SEM) == -1) {
		fprintf(stderr, "initSemInUse error");
		exit(EXIT_FAILURE);
	}

	/* Create/open backing file for mmap-based shared memory */
	fdshm = open(shmfile, O_RDWR | O_CREAT, 0666);
	if (fdshm == -1) {
		perror("open shmfile");
		exit(EXIT_FAILURE);
	}

	if (ftruncate(fdshm, sizeof(struct shmseg)) == -1) {
		perror("ftruncate shmfile");
		close(fdshm);
		exit(EXIT_FAILURE);
	}

	shmp = mmap(NULL, sizeof(struct shmseg), PROT_READ | PROT_WRITE, MAP_SHARED, fdshm, 0);
	if (shmp == MAP_FAILED) {
		perror("mmap shmfile");
		close(fdshm);
		exit(EXIT_FAILURE);
	}

	/* Transfer blocks of data from stdin to shared memory */
	for (transfers = 0, bytes = 0; ; transfers++, bytes += shmp->cnt) {
		if (reserveSem(semid, WRITE_SEM) == -1) { /* Wait for our turn */
			fprintf(stderr, "reserveSem error");
			exit(EXIT_FAILURE);
		}

		shmp->cnt = read(STDIN_FILENO, shmp->buf, BUF_SIZE);

		if (shmp->cnt == -1) {
			fprintf(stderr, "read error");
			exit(EXIT_FAILURE);
		}

		if (releaseSem(semid, READ_SEM) == -1) { /* Give reader a turn */
			fprintf(stderr, "releaseSem error");
			exit(EXIT_FAILURE);
		}

		if (shmp->cnt == 0) {
			break;
		}
	}

	/* Wait until reader has let us have one more turn. We then know
	reader has finished, and so we can delete the IPC objects. */
	if (reserveSem(semid, WRITE_SEM) == -1) {
		fprintf(stderr, "reserveSem error");
		exit(EXIT_FAILURE);
	}

	if (semctl(semid, 0, IPC_RMID, dummy) == -1) {
		fprintf(stderr, "semctl error");
		exit(EXIT_FAILURE);
	}

	if (munmap(shmp, sizeof(struct shmseg)) == -1) {
		fprintf(stderr, "munmap error");
	}
	close(fdshm);
	/* Optionally remove backing file */
	unlink(shmfile);

	printf("Sent %d bytes (%d transfers)\n", bytes, transfers);
	exit(EXIT_SUCCESS);
}
