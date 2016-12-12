#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <sys/time.h>

int reads = 0;

void close_signal_handler(int sig)
{
    printf("Completed %d reads in 5 seconds\n", reads);
    exit(0);
}

int main(int argc, char const *argv[])
{
    FILE *fp = fopen(argv[1], "rb");
    int last_int = sizeof(int) * (100 - 1);
    struct itimerval timer;
    int n;

    timer.it_value.tv_sec = 5;
    timer.it_value.tv_usec = 0;

    signal(SIGVTALRM, close_signal_handler);
    signal(SIGALRM, close_signal_handler);
    setitimer(ITIMER_VIRTUAL, &timer, NULL);

    sigset_t waiting_mask;

    while (++reads) {
        fseek(fp, rand() % last_int, SEEK_SET);
        fread(&n, 1, sizeof(int), fp);
    }

    fclose(fp);

    return 0;
}
