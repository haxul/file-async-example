#ifndef ASYNC_FILE_IO_H
#define ASYNC_FILE_IO_H

#include <aio.h>
#include <stdio.h>


struct aiocb* async_write(FILE* fp, char* buf, int buf_size);

struct aiocb* async_read(FILE* fp, char* buf, int buf_size);

void wait_aio(const struct aiocb* aio);


#endif //ASYNC_FILE_IO_H
