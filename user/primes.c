#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"

void calc(int in) {
    int prime, x, p[2];
    if (read(in, &prime, 4) != 0) {
        printf("prime %d\n", prime);
        pipe(p);
        if (fork() > 0) {
            close(p[0]);
            while (read(in, &x, 4) != 0) {
                if (x % prime == 0) continue;
                write(p[1], &x, 4);
            }
            close(p[1]);
            wait((int*)0);
        } else {
            close(p[1]);
            calc(p[0]);
            close(p[0]);
        }
    }
}

int main(int argc, char *argv[]) {
    int p[2];
    pipe(p);
    if (fork() > 0) {
        close(p[0]);
        for (int i = 2; i <= 35; ++i)
            write(p[1], &i, 4);
        close(p[1]);
        wait((int*)0);
    } else {
        close(p[1]);
        calc(p[0]);
        close(p[0]);
    }
    exit(0);
}
