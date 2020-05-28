#include <errno.h>
#include <poll.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/inotify.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>

void lock_system()
{
    if (access("/usr/bin/loginctl", X_OK) == 0)
        system("/usr/bin/loginctl lock-sessions");
    else
    if (access("/usr/bin/vlock", X_OK) == 0)
        system("/usr/bin/vlock");
}

void handle_events(int fd, int *wd)
{
    char buf[4096]
       __attribute__ ((aligned(__alignof__(struct inotify_event))));
    const struct inotify_event *event;
    int i;
    ssize_t len;
    char *ptr;

    while (1) {
        len = read(fd, buf, sizeof buf);
        if (len == -1 && errno != EAGAIN) {
            perror("read");
            exit(EXIT_FAILURE);
        }

        if (len <= 0)
            break;

        for (ptr = buf; ptr < buf + len; ptr += sizeof(struct inotify_event) + event->len) {
            event = (const struct inotify_event *) ptr;
            if ((event->len) && (!(event->mask & IN_ISDIR))) {
                int fd;
               
                usleep(10000);
                fd = open("/var/log/messages", O_RDONLY);
                if (fd > 0) {
                    char buf[512];
                    char class[1024];
                    int size = lseek(fd, 0, SEEK_END);
                    lseek(fd, size - sizeof(buf), SEEK_SET);
                    memset(buf, 0, sizeof(buf));
                    size = read(fd, buf, sizeof(buf) - 1);
                    buf[size] = 0;
                    close(fd);
               
                    snprintf(class, sizeof(class), "input,%s", event->name);
                    if ((strstr(buf, class) != NULL) && (strstr(buf, "Keyboard") != NULL)) {
                        lock_system();
                    }
                    if ((strstr(buf, class) != NULL) && (strstr(buf, "Mouse") != NULL)) {
                        lock_system();
                    }
                }
            }
        }
    }
}

int main(int argc, char* argv[])
{
    char buf;
    int fd, i, poll_num;
    int *wd;
    nfds_t nfds;
    struct pollfd fds[1];

    fd = inotify_init1(IN_NONBLOCK);
    if (fd == -1) {
        perror("inotify_init1");
        exit(EXIT_FAILURE);
    }

    wd = calloc(argc, sizeof(int));
    if (wd == NULL) {
        perror("calloc");
        exit(EXIT_FAILURE);
    }

    wd[i] = inotify_add_watch(fd, "/dev", IN_CREATE);
    if (wd[i] == -1) {
        fprintf(stderr, "Cannot watch '%s': %d\n",
                argv[i], -errno);
        exit(EXIT_FAILURE);
    }

    nfds = 1;

    fds[0].fd = fd;
    fds[0].events = POLLIN;

    printf("Listening for events.\n");
    while (1) {
        poll_num = poll(fds, nfds, -1);
        if (poll_num == -1) {
            if (errno == EINTR)
                continue;
            perror("poll");
            exit(EXIT_FAILURE);
        }

        if (poll_num > 0) {
            if (fds[0].revents & POLLIN) {
                handle_events(fd, wd);
            }
        }
    }

    printf("Done.\n");

    close(fd);

    free(wd);
    exit(EXIT_SUCCESS);
}

