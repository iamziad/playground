#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/inotify.h>
#include <unistd.h>

struct recent_event {
    size_t wd_idx;
    uint32_t mask;
    uint32_t cookie;
    char name[256];
    char type[32];
};

static void recent_update(
    struct recent_event *re,
    const uint32_t mask,
    const uint32_t cookie,
    const size_t wd_idx,
    const char name[],
    const char type[]
)
{
    re->mask = mask;
    re->cookie = cookie;
    re->wd_idx = wd_idx;
    strncpy(re->name, name, sizeof re->name);
    strncpy(re->type, type, sizeof re->type);
    re->name[sizeof re->name - 1] = '\0';
    re->type[sizeof re->type - 1] = '\0';
}

int main(int argc, char *argv[])
{
    if (argc < 2) {
        fprintf(stderr, "Usage: %s [directory]\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    int fd = inotify_init();

    if (fd < 0) {
        perror("Error");
        exit(EXIT_FAILURE);
    }

    struct recent_event recent = {0};
    struct inotify_event *event;
    int wdirs = argc - 1;
    int wd[wdirs];
    char buf[4096];
    size_t size;

    for (size_t i = 0; i < wdirs; i++) {
        wd[i] = inotify_add_watch(
            fd,
            argv[i + 1],
            IN_CREATE | IN_DELETE | IN_CLOSE_WRITE | IN_MOVE_SELF |
                IN_MOVED_FROM | IN_MOVED_TO | IN_DELETE_SELF
        );

        if (wd[i] < 0) {
            perror("Error");
            exit(EXIT_FAILURE);
        }
    }

    printf("Watching ");
    for (size_t i = 1; i < argc; i++) {
        printf("%s ", argv[i]);
    }
    printf("\n\n");

    while (1) {
        size = read(fd, buf, sizeof(buf));

        if (size == -1) {
            perror("Error");
            exit(EXIT_FAILURE);
        }

        size_t wd_idx = -1;

        for (char *ptr = buf; ptr < buf + size;
             ptr += sizeof(struct inotify_event) + event->len) {
            event = (struct inotify_event *)ptr;

            for (size_t i = 0; i < wdirs; i++)
                if (wd[i] == event->wd) {
                    wd_idx = i + 1;
                    break;
                }

            if (wd_idx == -1)
                continue;

            const char *type =
                (event->mask & IN_ISDIR) ? "[DIRECTORY]" : "[FILE]";

            if (event->mask & IN_CREATE)
                printf(
                    "%s/%s is created %s\n", argv[wd_idx], event->name, type
                );

            if (event->mask & IN_CLOSE_WRITE)
                printf(
                    "%s/%s is touched %s\n", argv[wd_idx], event->name, type
                );

            if (event->mask & IN_DELETE)
                printf(
                    "%s/%s is deleted %s\n", argv[wd_idx], event->name, type
                );

            if (event->mask & IN_DELETE_SELF) {
                printf("WATCHED DIRECTORY IS DELETED\n");
                if (argc == 2)
                    exit(EXIT_FAILURE);
            }

            if (event->mask & IN_MOVED_FROM) {
                recent_update(
                    &recent,
                    event->mask,
                    event->cookie,
                    wd_idx,
                    event->name,
                    type
                );
            }

            if (event->mask & IN_MOVED_TO) {
                if (event->cookie == recent.cookie) {
                    if (strcmp(argv[wd_idx], argv[recent.wd_idx]) == 0)
                        printf(
                            "%s/%s is renamed to %s %s\n",
                            argv[wd_idx],
                            recent.name,
                            event->name,
                            type
                        );
                    else
                        printf(
                            "%s/%s is moved to %s/%s\n",
                            argv[recent.wd_idx],
                            recent.name,
                            argv[wd_idx],
                            event->name
                        );
                } else
                    printf(
                        "%s is moved to (%s) from unwatched directory %s\n",
                        event->name,
                        argv[wd_idx],
                        type
                    );

                recent.cookie = 0;
            }

            if (event->mask & IN_MOVE_SELF)
                printf(
                    "The watched directory (%s) itself was moved %s\n",
                    event->name,
                    type
                );
        }

        if (recent.cookie > 0) {
            printf(
                "%s/%s is moved outside watched directory "
                "%s\n",
                argv[wd_idx],
                recent.name,
                recent.type
            );
        }
    }

    return EXIT_SUCCESS;
}
