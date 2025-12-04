#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <sys/stat.h>

#define BUF_SIZE 8192

int main(int argc, char *argv[])
{
    if (argc != 3) {
        fprintf(stderr, "Usage: %s src dest\n", argv[0]);
        return EXIT_FAILURE;
    }

    const char *src = argv[1];
    const char *dest = argv[2];

    int fdsrc = open(src, O_RDONLY);
    if (fdsrc == -1) {
        perror("open src");
        return EXIT_FAILURE;
    }

    /* Create dest with same permissions as src if possible */
    struct stat st;
    mode_t mode = 0644;
    if (fstat(fdsrc, &st) == 0) {
        mode = st.st_mode & 0777;
    }

    int fddest = open(dest, O_WRONLY | O_CREAT | O_TRUNC, mode);
    if (fddest == -1) {
        perror("open dest");
        close(fdsrc);
        return EXIT_FAILURE;
    }

    ssize_t r;
    char buf[BUF_SIZE];
    while ((r = read(fdsrc, buf, sizeof(buf))) > 0) {
        char *out_ptr = buf;
        ssize_t to_write = r;
        while (to_write > 0) {
            ssize_t w = write(fddest, out_ptr, to_write);
            if (w == -1) {
                if (errno == EINTR) continue;
                perror("write dest");
                close(fdsrc);
                close(fddest);
                return EXIT_FAILURE;
            }
            to_write -= w;
            out_ptr += w;
        }
    }
    if (r == -1) {
        perror("read src");
    }

    close(fdsrc);
    close(fddest);

    return (r == -1) ? EXIT_FAILURE : EXIT_SUCCESS;
}
