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
#include <sys/socket.h>
#include <fcntl.h>
#include <event2/event.h>
#include <event2/buffer.h>
#include <event2/bufferevent.h>
#include <assert.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <asm-generic/socket.h>
#define BUFF_SIZE 4 * 1024

#define MAX_LINE 16384

void do_read(evutil_socket_t fd, short events, void* arg);

void do_write(evutil_socket_t fd, short events, void* arg);

char rot13_char(char c) {
    /* We don't want to use isalpha here; setting the locale would change
     * which characters are considered alphabetical. */
    if ((c >= 'a' && c <= 'm') || (c >= 'A' && c <= 'M'))
        return c + 13;
    if ((c >= 'n' && c <= 'z') || (c >= 'N' && c <= 'Z'))
        return c - 13;
    return c;
}

void readcb(struct bufferevent* bev, void* ctx) {
    char* line;
    size_t n;
    int i;
    struct evbuffer* input = bufferevent_get_input(bev);
    struct evbuffer* output = bufferevent_get_output(bev);
    n = evbuffer_get_length(input);
    char* data = malloc(n);
    if (data == NULL) {
        perror("cannot allocate memory");
        return;
    }

    evbuffer_copyout(input, data, n);
    evbuffer_add_buffer(output, input);
    free(data);
    /*
        while ((line = evbuffer_readln(input, &n, EVBUFFER_EOL_LF))) {
            for (i = 0; i < n; ++i) {
                line[i] = rot13_char(line[i]);
            }
            evbuffer_add(output, line, n);
            evbuffer_add(output, "\n", 1);
            free(line);
        }

        if (evbuffer_get_length(input) >= MAX_LINE) {
            /* Too long; just process what there is and go on so that the buffer
             * doesn't grow infinitely long. */
    /*        char buf[1024];
            while (evbuffer_get_length(input)) {
                int n = evbuffer_remove(input, buf, sizeof(buf));
                for (i = 0; i < n; ++i) {
                    buf[i] = rot13_char(buf[i]);
                }
                evbuffer_add(output, buf, n);
            }
            evbuffer_add(output, "\n", 1);
        }
    */
}

void errorcb(struct bufferevent* bev, short error, void* ctx) {
    if (error & BEV_EVENT_EOF) {
        /* connection has been closed, do any clean up here */
        /* ... */
    }
    else if (error & BEV_EVENT_ERROR) {
        /* check errno to see what error occurred */
        /* ... */
    }
    else if (error & BEV_EVENT_TIMEOUT) {
        /* must be a timeout event handle, handle it */
        /* ... */
    }
    bufferevent_free(bev);
}

void do_accept(evutil_socket_t listener, short event, void* arg) {
    struct event_base* base = arg;
    struct sockaddr_storage ss;
    socklen_t slen = sizeof(ss);
    int fd = accept(listener, (struct sockaddr *)&ss, &slen);
    if (fd < 0) {
        perror("accept");
    }
    else if (fd > FD_SETSIZE) {
        close(fd);
    }
    else {
        struct bufferevent* bev;
        evutil_make_socket_nonblocking(fd);
        bev = bufferevent_socket_new(base, fd, BEV_OPT_CLOSE_ON_FREE);
        bufferevent_setcb(bev, readcb, NULL, errorcb, NULL);
        bufferevent_setwatermark(bev, EV_READ, 0, MAX_LINE);
        bufferevent_enable(bev, EV_READ | EV_WRITE);
    }
}

void run() {
    struct event_base* base = event_base_new();
    if (!base) {
        return; /*XXXerr*/
    }

    struct sockaddr_in sin;
    sin.sin_family = AF_INET;
    sin.sin_addr.s_addr = 0;
    sin.sin_port = htons(3031);

    const int listener = socket(AF_INET, SOCK_STREAM, 0);
    const int optval = 1;
    setsockopt(listener, SOL_SOCKET, SO_REUSEPORT, &optval, sizeof(optval));
    evutil_make_socket_nonblocking(listener);

    if (bind(listener, (struct sockaddr *)&sin, sizeof(sin)) < 0) {
        perror("bind");
        return;
    }

    if (listen(listener, 16) < 0) {
        perror("listen");
        return;
    }

    struct event* listener_event = event_new(base, listener, EV_READ | EV_PERSIST, do_accept, base);
    /*XXX check it */
    event_add(listener_event, NULL);

    event_base_dispatch(base);
}

int main() {
    setvbuf(stdout, NULL, _IONBF, 0);

    run();
    return 0;
}
