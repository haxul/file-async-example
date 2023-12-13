#define _POSIX_SOURCE
#include <aio.h>
#include <event2/buffer.h>
#include <event2/event.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>
#include <sched.h>
#include "src/async_file_io.h"

#define BUFF_SIZE 4 * 1024

int main() {
    FILE* const fp =
            fopen("/home/haxul/Development/clanguage/kv-storage/text.txt", "w");
    if (fp == NULL) {
        return 1;
    }
    char* buf = "something new";
    struct aiocb* aio = async_write(fp, buf, 13);

    wait_aio(aio);

    const int ret = aio_return(aio);
    printf("result is %d\n", ret);
    printf("%s\n", buf);
    fclose(fp);
    free(aio);
    return 0;
}
