#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"

int main(int argc, char *argv[]) {
    int p[2], q[2];
    pipe(p);
    pipe(q);
    if (fork() > 0) {
        close(p[0]);
        close(q[1]);
        write(p[1], "s", 2);
        char buf;
        if (read(q[0], &buf, 1) != 1) {
            fprintf(2, "pingping: parent read error.\n");
            exit(1);
        }
        printf("%d: received pong\n", getpid());
    } else {
        char buf;
        close(p[1]);
        close(q[0]);
        if (read(p[0], &buf, 1) != 1) {
            fprintf(2, "pingping: child read error.\n");
            exit(1);
        }
        printf("%d: received ping\n", getpid());
        write(q[1], &buf, 2);
    }
    exit(0);
}
