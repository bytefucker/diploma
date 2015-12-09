//      memRand.c
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

#include<stdio.h>
#include<stdlib.h>
#include<sys/time.h>
#include<pthread.h>
#include<string.h>

#define NUM_THREADS 4
#define NUM_BLOCKS 10485760 /* 1024*1024*10 */

int 	choice;
long	start_time[NUM_THREADS];
long	end_time  [NUM_THREADS];

/* Prototypes */
void *rand_byte(void *);
void *rand_kb(void *);
void *rand_mb(void *);

void calculateTime(int num_bytes)
{
	double time_sec, time_computed;
	long start, end, i;
	start = start_time[0];
	end = end_time[0];
	for (i = 0; i < NUM_THREADS; i++)
	{
		if (start_time[i] < start)
			start = start_time[i];
		if (end_time[i] > end)
			end = end_time[i];
	}
	time_sec = (double)(end - start) / 1000000.0;

	printf("Time for %d threads of %d bytes is %lf seconds\n", NUM_THREADS, num_bytes, time_sec);
	printf("Speed is %lf MB/sec\n", (double)(NUM_BLOCKS) / (time_sec * 1024 * 1024));
}

int main (int argc, char ** argv)
{
	double time;

	printf("Random Access Mode:\n");
	pthread_t threads[NUM_THREADS];
	int rc;
	long t;
	
	for (t = 0; t < NUM_THREADS; t++) {
			rc = pthread_create(&threads[t], NULL, rand_byte, (void *)t);
			if (rc) {
				printf("ERROR; return code from pthread_create() is %d\n", rc);
				exit(-1);
			}
		}
		for (t = 0; t < NUM_THREADS; t++) {
			pthread_join(threads[t], NULL);
		}

		calculateTime(1);

		for (t = 0; t < NUM_THREADS; t++) {
			rc = pthread_create(&threads[t], NULL, rand_kb, (void *)t);
			if (rc) {
				printf("ERROR; return code from pthread_create() is %d\n", rc);
				exit(-1);
			}
		}
		for (t = 0; t < NUM_THREADS; t++) {
			pthread_join(threads[t], NULL);
		}

		calculateTime(1024);

		for (t = 0; t < NUM_THREADS; t++) {
			rc = pthread_create(&threads[t], NULL, rand_mb, (void *)t);
			if (rc) {
				printf("ERROR; return code from pthread_create() is %d\n", rc);
				exit(-1);
			}
		}
		for (t = 0; t < NUM_THREADS; t++) {
			pthread_join(threads[t], NULL);
		}

		calculateTime(1048576);
}

void *rand_byte(void *param)
{
	struct timeval start, stop;
	long long i, k;
	long tid = (long)param;
	char *array1, *array2, *temp;
	long long M = 0, N = (NUM_BLOCKS / NUM_THREADS);
	array1 = (char *)malloc(NUM_BLOCKS / NUM_THREADS);
	array2 = (char *)malloc(NUM_BLOCKS / NUM_THREADS);
	memset(array2, 'a', NUM_BLOCKS / NUM_THREADS);
	gettimeofday(&start, NULL);
	start_time[tid] = (start.tv_sec * 1000000) + start.tv_usec;
	for (i = 0; i < (NUM_BLOCKS / NUM_THREADS); i++) {
		//k = rand() % 1048576;
		srand((unsigned)time(NULL));
		k = M + rand() / (RAND_MAX / (N - M + 1) + 1);
		memcpy(array1 + k, array2 + k, 1);
	}

	gettimeofday(&stop, NULL);
	end_time[tid] = (stop.tv_sec * 1000000) + stop.tv_usec;
	free(array2);
	free(array1);

}

void *rand_kb(void *param)
{
	struct timeval	start, stop;
	long long i, k;
	long tid = (long)param;
	long long M = 0, N = (NUM_BLOCKS / (1024 * NUM_THREADS));
	char *array1, *array2, *temp;
	array1 = (char *)malloc(NUM_BLOCKS / NUM_THREADS);
	array2 = (char *)malloc(NUM_BLOCKS / NUM_THREADS);
	memset(array2, 'a', NUM_BLOCKS / NUM_THREADS);
	gettimeofday(&start, NULL);
	start_time[tid] = (start.tv_sec * 1000000) + start.tv_usec;
	for (i = 0; i < (NUM_BLOCKS / (1024 * NUM_THREADS)); i++) {
		//k = rand() % 20479;
		srand((unsigned)time(NULL));
		k = M + rand() / (RAND_MAX / (N - M + 1) + 1);
		memcpy(array1 + (k), array2 + (k * 1024), 1024);
	}
	gettimeofday(&stop, NULL);
	end_time[tid] = (stop.tv_sec * 1000000) + stop.tv_usec;
	free(array2);
	free(array1);
}


void *rand_mb(void *param)
{
	struct timeval	start, stop;
	long long i, k;
	long tid = (long)param;
	long long M = 0, N = NUM_BLOCKS / (1024 * 1024 * NUM_THREADS);
	char *array1, *array2, *temp1;
	array1 = (char *)malloc(NUM_BLOCKS / NUM_THREADS);
	array2 = (char *)malloc(NUM_BLOCKS / NUM_THREADS);
	memset(array2, 'a', (NUM_BLOCKS / NUM_THREADS));
	gettimeofday(&start, NULL);
	start_time[tid] = (start.tv_sec * 1000000) + start.tv_usec;
	for (i = 0; i < (NUM_BLOCKS / (1024 * 1024 * NUM_THREADS)); i++) {
		//k = rand() % 19;
		srand((unsigned)time(NULL));
		k = M + rand() / (RAND_MAX / (N - M + 1) + 1);
		memcpy(array1 + (k), array2 + (k * 1048576), 1048576);
	}

	gettimeofday(&stop, NULL);
	end_time[tid] = (stop.tv_sec * 1000000) + stop.tv_usec;
	free(array2);
	free(array1);
}
