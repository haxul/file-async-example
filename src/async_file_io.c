#define _POSIX_SOURCE
#include <aio.h>
#include <stdio.h>
#include <stdlib.h>
#include <sched.h>
#include <asm-generic/errno.h>

struct aiocb* init_aio(FILE* fp, char* buf, const int buf_size) {
    struct aiocb* const aio = calloc(1, sizeof(struct aiocb));
    if (aio == NULL) return NULL;
    aio->aio_buf = buf;
    aio->aio_fildes = fileno(fp);
    aio->aio_nbytes = buf_size;
    aio->aio_offset = 0;

    return aio;
}

struct aiocb* async_write(FILE* fp, char* buf, const int buf_size) {
    struct aiocb* const aio = init_aio(fp, buf, buf_size);
    if (aio == NULL) return NULL;
    const int res = aio_write(aio);
    if (res < 0) {
        free(aio);
        return NULL;
    }

    return aio;
}

struct aiocb* async_read(FILE* fp, char* buf, const int buf_size) {
    struct aiocb* const aio = init_aio(fp, buf, buf_size);

    const int res = aio_read(aio);

    if (res < 0) {
        free(aio);
        return NULL;
    }

    return aio;
}

void wait_aio(const struct aiocb* aio) {
    while (aio_error(aio) == EINPROGRESS) {
        sched_yield();
    }
}
