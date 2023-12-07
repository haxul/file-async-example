#define _POSIX_SOURCE
#include <netinet/in.h>
#include <sys/socket.h>
#include <fcntl.h>
#include <event2/event.h>
#include <event2/buffer.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <aio.h>

#define BUFF_SIZE 4 * 1024

struct aiocb* async_read(FILE* fp, char* buf, int buf_size) {
    struct aiocb* const aio = malloc(sizeof(struct aiocb));
    if (aio == NULL) {
        return NULL;
    }
    memset(aio, 0, sizeof(*aio));
    aio->aio_buf = buf;
    aio->aio_fildes = fileno(fp);
    aio->aio_nbytes = buf_size;
    aio->aio_offset = 0;

    const int res = aio_read(aio);

    if (res < 0) {
        free(aio);
        return NULL;
    }
    return aio;
}

int main() {
    FILE* const fp = fopen("/home/haxul/Development/clanguage/kv-storage/text.txt", "r");
    if (fp == NULL) {
        return 1;
    }
    char buf[BUFF_SIZE];

    struct aiocb* aio = async_read(fp, buf, BUFF_SIZE);

    uint64_t counter = 0;
    while (aio_error(aio) == EINPROGRESS) {
        counter++;
    }

    const int ret = aio_return(aio);
    printf("result is %d\n", ret);
    printf("%s\n", buf);
    printf("counter is %lu\n", counter);
    fclose(fp);
    return 0;
}
