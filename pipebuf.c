// gcc -D_GNU_SOURCE -pthread pipebuf.c && ./a.out 1234
#include <pthread.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>     // atoi
#include <string.h>     // memset...
#include <unistd.h>     // pipe

#define MSGSIZE      3072
#define NTHREADS     254
#define BUFSIZE   (65536+4096)

int pipev[2], msgsize = 0, done[NTHREADS];

static void *thr_send(void*arg)
{
    int  rc, thrid = (int)(intptr_t)arg;
    char snd[msgsize];
    rc = write(pipev[1], memset(snd, thrid, msgsize), msgsize);
    if (rc != msgsize) fprintf(stderr, "\nThread %d wrote: %d\n", thrid, rc);
    return arg;
}

static void epilogue(int i)
{
    fprintf(stderr, "\nThreads completed:\n");
    for (i = 0; i < NTHREADS; ++i) putc('0'+done[i], stderr);
    putc('\n', stderr);
    _exit(0);
}

int main(int argc, char **argv)
{
    if (argc == 2)  msgsize = atoi(argv[1]);
    if (!msgsize)   return fputs("Usage: pipebuf <msgsize>\n", stderr);
    setvbuf(stderr, NULL, _IOLBF, 0);
    fprintf(stderr, "%d threads writing msgs of %d byte until write blocks\n", NTHREADS, msgsize);

    pthread_t           tidv[NTHREADS];
    pthread_attr_t      attr;
    pthread_attr_init(&attr);
    pthread_attr_setdetachstate(&attr,PTHREAD_CREATE_DETACHED);

    int i, rc = pipe(pipev);
    for (i = rc = 0; rc >= 0 && i <NTHREADS; ++i)
        rc = pthread_create(&tidv[i], &attr, thr_send, (void*)(intptr_t)(i+1));
    if (rc < 0) return fprintf(stderr, "pthread_create failed: %d\n", rc);

    sleep(1);   // Let all threads start.
    signal(SIGALRM, epilogue);
    alarm(1);

    int totbytes = 0, need = msgsize, bufsize = NTHREADS*msgsize/4;
    if (bufsize > BUFSIZE) bufsize = BUFSIZE;
    unsigned char rcv[bufsize], test = 0;

    for (; 0 < (rc = read(pipev[0], rcv, bufsize)); totbytes += rc) {
        fprintf(stderr, "Received: %d/%d\n", rc, bufsize);
        for (i = 0; i < rc; i += msgsize) {
            int j = 0, more = need < rc - i ? need : rc - i;
            if (need == msgsize) test = rcv[i];
            while (j < more && rcv[i+j] == test) ++j;
            if (j < more) {
                fprintf(stderr, "Failed at %d + %d: exp=%u act=%u\n", totbytes, i+j, rcv[i+j], test);
                epilogue(0);
            }
            if ((need -= more)) test = rcv[rc - 1];
            else need = msgsize, done[test - 1] = 1;
        }
    }
    return  1; // read failed
}
