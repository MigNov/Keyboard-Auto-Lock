#include <errno.h>
#include <poll.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/inotify.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>

int debug = 0;
char last_line[1024];

int run_command(char *cmd)
{
    return (WEXITSTATUS( system(cmd) ) == 0) ? 1 : 0;
}

void lock_system()
{
    if (run_command("loginctl lock-sessions"))
        return;
    if (run_command("/bin/loginctl lock-sessions"))
        return;
    if (run_command("vlock"))
        return;
    if (run_command("/usr/bin/vlock"))
        return;
}

int is_required_dev(const char *dev)
{
    char cmd[1024];
    char buf[1024];
    FILE *fp = NULL;
    int ret = 0;
    char *ret_kbd;
    char *ret_mouse;

    snprintf(cmd, sizeof(cmd), "dmesg | grep \"input,%s\" | tail -n 1", dev);
    fp = popen(cmd, "r");
    if (fp == NULL) {
        return 0;
    }
    memset(buf, 0, sizeof(buf));
    fgets(buf, sizeof(buf), fp);
    fclose(fp);

    if (strcmp(last_line, buf) == 0) {
        return 0;
    }

    ret_kbd = strstr(buf, "Keyboard");
    ret_mouse = strstr(buf, "Mouse");
    if (ret_kbd || ret_mouse) {
        ret = 1;
        strncpy(last_line, buf, sizeof(last_line));
    }
    return ret;
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

                if (debug)
                    printf("Found a new entry in /dev tree: %s\n", event->name);

                int remaining_iterations = 50;
                while ( remaining_iterations > 0 ) {
                    if (is_required_dev(event->name)) {
                        if (debug)
                            printf("Device '%s' triggering lock\n", event->name);
                        lock_system();
                    }
                    usleep(10000);
                    remaining_iterations--;
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

    if (getuid() != 0) {
        printf("Error: You have to run this utility as root\n");
        return 1;
    }

    if ((argc > 1) && (strcmp(argv[1], "--debug") == 0)) {
        debug = 1;
    }

    fd = inotify_init1(IN_NONBLOCK);
    if (fd == -1) {
        perror("inotify_init1");
        exit(EXIT_FAILURE);
    }

    if (debug)
        printf("iNotify handler initialized\n");

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

    if (debug)
        printf("iNotify watcher on /dev established\n");

    nfds = 1;

    fds[0].fd = fd;
    fds[0].events = POLLIN;

    if (debug)
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

    if (debug)
        printf("Done.\n");

    close(fd);

    free(wd);
    exit(EXIT_SUCCESS);
}

