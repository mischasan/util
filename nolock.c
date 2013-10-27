// Copyright (C) 2009-2013 Mischa Sandberg <mischasan@gmail.com>
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License Version 2 as
// published by the Free Software Foundation.  You may not use, modify or
// distribute this program under any other version of the GNU General
// Public License.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
//
// IF YOU ARE UNABLE TO WORK WITH GPL2, CONTACT ME.
//-------------------------------------------------------------------

// Test to proof lock-free update across processes in an mmap'd file.

#include "msutil.h"
#include <fcntl.h>      // open
#include <pthread.h>
#include <sys/mman.h>
#include <sys/wait.h>
#include <sys/ipc.h>
#include <sys/sem.h>

int nvals = 16384, nreps = 10;
enum { UNK, CAS, RAW, SEM } mode;
int semid;

static void *doit(void*_map);
static void incr(int *vec, int pos);

int main(int argc, char **argv)
{
    mode = argc == 1 ? UNK 
         : !strcmp(argv[1], "cas") ? CAS 
         : !strcmp(argv[1], "raw") ? RAW 
         : !strcmp(argv[1], "sem") ? SEM : UNK;
    if (mode == UNK) usage("cas | raw | sem");

    char const *fname = "nolock.dat";
    if (mode == SEM) {
        semid = semget(0, 1, IPC_CREAT | IPC_PRIVATE | 0600);
        if (semid < 0) die(": cannot semget:");
    }

    int nconcs = 4, nthrs = 256;

    int nbytes = nvals * sizeof(int);
    int fd = open(fname, O_CREAT|O_RDWR, 0666);
    if (fd < 0) die(": cannot create %s:", fname);
    if (ftruncate(fd, nbytes)) die(": cannot resize %s:", fname);
    int *map = mmap(NULL, nbytes, PROT_READ|PROT_WRITE, MAP_SHARED, fd, 0);
    if (map == MAP_FAILED) die(": cannot mmap %s:", fname);
    memset(map, 0, nbytes);

    int i, j, sum;
    double  t = tick();
    for (i = 0; i < nconcs; ++i) {
        if (!fork()) {
            for (i = 0; i < nreps; ++i) {
                for (j = 0; j < nvals; ++j) incr(map,j);
                while (--j >= 0) incr(map,j);
            }
            exit(0);
        }
    }

    pthread_attr_t attr;
    pthread_attr_init(&attr);
    pthread_t tid[nthrs];
    void    *foo;

    for (i = 0; i < nthrs; ++i) pthread_create(&tid[i], &attr, doit, map);
    for (i = 0; i < nthrs; ++i) pthread_join(tid[i], &foo);

    for (i = 0; i < nconcs; ++i) wait(&j);
    t = tick() - t;
    for (i = sum = 0; i < nvals; ++i) sum += map[i];
    int exp = (nconcs + nthrs) * nreps * nvals * 2;
    printf("exp=%08X act=%08X %08X %.4f secs nvals=%d nreps=%d nthrs=%d nconcs=%d\n",
           exp, sum, exp - sum, t, nvals, nreps, nthrs, nconcs);

    if (mode == SEM && semctl(semid, 0, IPC_RMID)) die(": error destroying sem:");
    return 0;
}

static void *doit(void*_map)
{
    int *map = (int*)_map;
    int i, j;

    for (i = 0; i < nreps; ++i) {
        for (j = 0; j < nvals; ++j) incr(map,j);
        while (--j >= 0) incr(map,j);
    }

    return  NULL;
}

static void incr(int *vec, int pos)
{
    switch (mode) {
    case CAS:
        while (!__sync_bool_compare_and_swap(&vec[pos], vec[pos], vec[pos]+1));
        break;

    case RAW:
        ++vec[pos]; // prove that the simple increment doesn't work.
        break;

    case SEM:
        {
        struct sembuf sem = { pos & 15, -1, 0 }; // decr = lock
        if (semop(semid, &sem, 1)) die(": cannot acquire sem:");
        ++vec[pos];
        sem.sem_op = 1; // incr = unlock
        semop(semid, &sem, 1);
        }
        break;

    case UNK:
        break;
    }
}
