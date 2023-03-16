#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"
#include "kernel/param.h"


int main(int argc, char *argv[]) {
    char buf[512];
    char* args[MAXARG], *p = buf;
    for (int i = 0; i < argc - 1; ++i) args[i] = argv[i + 1];
    args[argc - 1] = 0;
    int cnt = argc - 1;
    while (read(0, p, 1) == 1) {
        if (*p == '\n' || *p == ' ') {
            char *q = p;
            while (q >= buf && *q) --q;
            if (q + 1 != p) args[cnt++] = q + 1;
        }
        if (*p == '\n') {
            *p = 0;
            // for (int i = 0; i < cnt; ++i) printf("%d(%d): %s\n", i, strlen(args[i]), args[i]);
            if (fork() > 0) wait(0);
            else exec(args[0], args);
            p = buf;
            cnt = argc - 1;
            continue;
        }
        if (*p == ' ') *p = 0;
        ++p;
    }
    exit(0);
}
