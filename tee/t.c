#include <stdio.h>

int main(int argc, char *argv[])
{
    if (argc != 2) {
        fprintf(stderr, "Usage: %s <filename>\n", argv[0]);
        return 1;
    }

    FILE *f = fopen(argv[1], "w");

    if (f == NULL) {
        fprintf(stderr, "%s: couldn't open file\n", argv[0]);
        return 1;
    }

    int c;
    while ((c = getchar()) != EOF) {
        putchar(c);
        putc(c, f);
    }
    putchar('\n');

    return 0;
}
