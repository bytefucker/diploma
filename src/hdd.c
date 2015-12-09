//      hdd.c
//
//      Copyright 2015 alex <sasha.p@hush.com>
//
//      This program is free software; you can redistribute it and/or modify
//      it under the terms of the GNU General Public License as published by
//      the Free Software Foundation; either version 2 of the License, or
//      (at your option) any later version.
//
//      This program is distributed in the hope that it will be useful,
//      but WITHOUT ANY WARRANTY; without even the implied warranty of
//      MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//      GNU General Public License for more details.
//
//      You should have received a copy of the GNU General Public License
//      along with this program; if not, write to the Free Software
//      Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,
//      MA 02110-1301, USA.

#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <errno.h>
#include <sys/time.h>
#include <time.h>
#include <fcntl.h>

#define SECONDS 10
#define MAX_BUFSIZE 8192

/* This must be set to the smallest BUFSIZE or 1024, whichever is smaller */
#define COUNTSIZE 256
#define HALFCOUNT (COUNTSIZE/2)         /* Half of COUNTSIZE */

#define FNAME0  "dummy0"
#define FNAME1  "dummy1"

/* ANSI/VT100 magical clear */
#define clear() printf("\033[H\033[J")

extern void sync(void);

/* Prototypes */
int w_test (int timeSecs);
int r_test (int timeSecs);
int c_test (int timeSecs);
void print_usage();

long read_score = 1, write_score = 1, copy_score = 1;

/****************** GLOBALS ***************************/

/* The buffer size for the tests. */
int bufsize = 1024;

/*
 * The max number of 1024-byte blocks in the file.
 * Don't limit it much, so that memory buffering
 * can be overcome.
 */
int max_blocks = 2000;

/* The max number of BUFSIZE blocks in the file. */
int max_buffs = 2000;

/* Countable units per 1024 bytes */
int count_per_k;

/* Countable units per bufsize */
int count_per_buf;

/* The actual buffer. */
/* char *buf = 0; */
char buf[MAX_BUFSIZE];

int                     f;
int                     g;
int                     i;
void                    stop_count();
void                    clean_up();
int                     sigalarm = 0;

/******************** MAIN ****************************/

int main (int argc, char ** argv)
{
    /* The number of seconds to run for. */
    int seconds = SECONDS;

    /* Cheat to display usage */
    char test = '.';

    int status;
    int i;

    for (i = 1; i < argc; ++i) {
        if (argv[i][0] == '-') {
            switch (argv[i][1]) {
                case 'w':
                case 'r':
                case 'c':
                case '.':
                    test = argv[i][1];
                    break;
                case 'b':
                    bufsize = atoi(argv[++i]);
                    break;
                case 'l':
                    max_blocks = atoi(argv[++i]);
                    break;
                case 't':
                    seconds = atoi(argv[++i]);
                    break;
                case 'd':
                    if (chdir(argv[++i]) < 0) {
                        perror("hdd: chdir");
                        exit(1);
                    }
                    break;
                default:
                    print_usage();
                    exit(2);
            }
        }
    }

	/* Some triggers */
    if (bufsize < COUNTSIZE || bufsize > MAX_BUFSIZE) {
        fprintf(stderr, "hdd: buffer size must be in range %d-%d\n",
                COUNTSIZE, 1024*1024);
        exit(3);
    }
    if (max_blocks < 1 || max_blocks > 1024*1024) {
        fprintf(stderr, "hdd: max blocks must be in range %d-%d\n",
                1, 1024*1024);
        exit(3);
    }
    if (seconds < 1 || seconds > 3600) {
        fprintf(stderr, "hdd: time must be in range %d-%d seconds\n",
                1, 3600);
        exit(3);
    }

    max_buffs = max_blocks * 1024 / bufsize;
    count_per_k = 1024 / COUNTSIZE;
    count_per_buf = bufsize / COUNTSIZE;

	/*Error handlers on files */
    if((f = creat(FNAME0, 0600)) == -1) {
            perror("hdd: creat");
            exit(1);
    }
    close(f);

    if((g = creat(FNAME1, 0600)) == -1) {
            perror("hdd: create");
            exit(1);
    }
    close(g);

    if( (f = open(FNAME0, 2)) == -1) {
            perror("hdd: open");
            exit(1);
    }
    if( ( g = open(FNAME1, 2)) == -1 ) {
            perror("hdd: open");
            exit(1);
    }

    /* fill buffer */
    for (i=0; i < bufsize; ++i)
            buf[i] = i & 0xff;

    signal(SIGKILL,clean_up);

    switch (test) {
    case 'w':
        status = w_test(seconds);
        break;
    case 'r':
        w_test(2);
        status = r_test(seconds);
        break;
    case 'c':
        w_test(2);
        r_test(2);
        status = c_test(seconds);
        break;
    case '.':
	print_usage();
        break;
    default:
        fprintf(stderr, "hdd: unknown test \'%c\'\n", test);
        exit(6);
    }
    if (status) {
        clean_up();
        exit(1);
    }

    clean_up();
    exit(0);
}

/* Get time */
static double getFloatTime()
{
	struct timeval t;
	gettimeofday(&t, 0);
	return (double) t.tv_sec + (double) t.tv_usec / 1000000.0;
}

/* Run the write test for the time given in seconds */
int w_test(int timeSecs)
{
        unsigned long counted = 0L;
        unsigned long tmp;
        long f_blocks;
        double start, end;
        extern int sigalarm;

        /* Sync and let it settle */
        sync();
        sleep(2);
        sync();
        sleep(2);

        /* Set an alarm. */
        sigalarm = 0;
        signal(SIGALRM, stop_count);
        alarm(timeSecs);

        start = getFloatTime();

        while (!sigalarm) {
                for(f_blocks=0; f_blocks < max_buffs; ++f_blocks) {
                        if ((tmp=write(f, buf, bufsize)) != bufsize) {
                                if (errno != EINTR) {
                                        perror("hdd: write");
                                        return(-1);
                                }
                                stop_count();
                                counted += ((tmp+HALFCOUNT)/COUNTSIZE);
                        } else
                                counted += count_per_buf;
                }
                lseek(f, 0L, 0); /* rewind */
        }

        /* stop clock */
        end = getFloatTime();
        write_score = (long) ((double) counted / ((end - start) * count_per_k));
        printf("Write done: %ld in %.4f\n", counted, end - start);
        printf("Speed: %ld KBps\n", write_score);
        printf("Time: %.1f\n\n", end - start);

        return(0);
}

/* Run the read test for the time given in seconds */
int r_test(int timeSecs)
{
        unsigned long counted = 0L;
        unsigned long tmp;
        double start, end;
        extern int sigalarm;
        extern int errno;

        /* Sync and let it settle */
        sync();
        sleep(2);
        sync();
        sleep(2);

        /* rewind */
        errno = 0;
        lseek(f, 0L, 0);

        /* Set an alarm. */
        sigalarm = 0;
        signal(SIGALRM, stop_count);
        alarm(timeSecs);

        start = getFloatTime();

        while (!sigalarm) {
                /* read while checking for an error */
                if ((tmp=read(f, buf, bufsize)) != bufsize) {
                        switch(errno) {
                        case 0:
                        case EINVAL:
                                lseek(f, 0L, 0);  /* rewind at end of file */
                                counted += (tmp+HALFCOUNT)/COUNTSIZE;
                                continue;
                        case EINTR:
                                stop_count();
                                counted += (tmp+HALFCOUNT)/COUNTSIZE;
                                break;
                        default:
                                perror("hdd: read");
                                return(-1);
                                break;
                        }
                } else
                        counted += count_per_buf;
        }

        /* stop clock */
        end = getFloatTime();
        read_score = (long) ((double) counted / ((end - start) * count_per_k));
        printf("Read done: %ld in %.4f\n", counted, end - start);
        printf("Speed: %ld KBps\n", read_score);
        printf("Time: %.1f\n\n", end - start);

        return(0);
}


/* Run the copy test for the time given in seconds */
int c_test(int timeSecs)
{
        unsigned long counted = 0L;
        unsigned long tmp;
        double start, end;
        extern int sigalarm;

        sync();
        sleep(2);
        sync();
        sleep(1);

        /* rewind */
        errno = 0;
        lseek(f, 0L, 0);

        /* Set an alarm. */
        sigalarm = 0;
        signal(SIGALRM, stop_count);
        alarm(timeSecs);

        start = getFloatTime();

        while (!sigalarm) {
                if ((tmp=read(f, buf, bufsize)) != bufsize) {
                        switch(errno) {
                        case 0:
                        case EINVAL:
                                lseek(f, 0L, 0);  /* rewind at end of file */
                                lseek(g, 0L, 0);  /* rewind the output too */
                                continue;
                        case EINTR:
                                /* part credit for leftover bytes read */
                                counted += ( (tmp * write_score) /
                                        (read_score + write_score)
                                        + HALFCOUNT) / COUNTSIZE;
                                stop_count();
                                break;
                        default:
                                perror("hdd: copy read");
                                return(-1);
                                break;
                        }
                } else  {
                        if ((tmp=write(g, buf, bufsize)) != bufsize) {
                                if (errno != EINTR) {
                                        perror("hdd: copy write");
                                        return(-1);
                                }
                                counted += (
                                 /* Full credit for part of buffer written */
                                        tmp +

                                 /* Plus part credit having read full buffer */
                                        ( ((bufsize - tmp) * write_score) /
                                        (read_score + write_score) )
                                        + HALFCOUNT) / COUNTSIZE;
                                stop_count();
                        } else
                                counted += count_per_buf;
                }
        }

        /* stop clock */
        end = getFloatTime();
        copy_score = (long) ((double) counted / ((end - start) * count_per_k));

        printf("Copy done: %ld in %.4f\n", counted, end - start);
        printf("Speed: %ld KBps\n", copy_score);
        printf("Time: %.1f\n\n", end - start);

        return(0);
}

void stop_count(void)
{
	extern int sigalarm;
	sigalarm = 1;
}

void clean_up(void)
{
	unlink(FNAME0);
	unlink(FNAME1);
}

void print_usage()
{
    printf("hdd is a simple tool for testing the hard drive.\nPossible options:\n-w\ttest write speed\n-r\ttest read speed\n-c\ttest copy speed\n-b\tbuffer size\n-t\ttest time\n-l\tmax block size\n-d\tchange dir\n-?\tshow this help\n");
}

