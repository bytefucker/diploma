//      info.c
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
#include <sys/time.h>
#include <time.h>

#define MAXLINELENGTH 1000

/* Prototypes */
int readAndParse (char * the_filename, char * search_token, int flag);
void printUptime (char* label, long time);

/******************** MAIN ****************************/

int main (int argc, char ** argv)
{
	printInfo();
}

/* Get time */
static double getFloatTime()
{
	struct timeval t;
	gettimeofday(&t, 0);
	return (double) t.tv_sec + (double) t.tv_usec / 1000000.0;
}

void printUptime (char* label, long time)
{
	const long minute = 60;
	const long hour = minute * 60;
	const long day = hour * 24;
	printf (" %s%02ld day(s) %02ld hour(s) %02ld minute(s)\n", label, time / day, (time % day) / hour, (time % hour) / minute);
	return;
}

int printInfo ()
{
	double uptime, idle_time;
	char hostname[MAXLINELENGTH];
	struct timeval now;
	struct timeval;
	FILE * fp;
	gettimeofday(&now, NULL);
	printf("date: %s\n", ctime(&(now.tv_sec)));

	/* Print CPU type and model */
	printf("vendor_id:\t\t");
	readAndParse("/proc/cpuinfo", "vendor_id", 0);
	printf("model name:\t\t");
	readAndParse("/proc/cpuinfo", "name", 0);
	printf ("CPU bogomips:\t\t");
	readAndParse("/proc/cpuinfo", "bogomips", 0);

	/* Print Linux kernel version */
	printf("kernel version:\t\t");
	readAndParse("/proc/version", "Linux", 1);

	/* Print time since last booted */
	printf("\nuptime:\t\t\t");
	fp = fopen ("/proc/uptime", "r");
	fscanf (fp, "%lf %lf\n", &uptime, &idle_time);
	fclose (fp);
	printUptime ("", (long) uptime);

	/* Print memory info */
	printf("Memory Total:\t\t");
	readAndParse("/proc/meminfo", "MemTotal:", 2);
	printf(" kB");
	printf("\nMemory Free:\t\t");
	readAndParse("/proc/meminfo", "MemFree:", 2);
	printf(" kB\n");


	return 0;
}

int readAndParse (char * the_filename, char * search_token, int flag){
FILE * file;
char line[MAXLINELENGTH];
if ((file = fopen(the_filename, "r")) == NULL)
	  printf("Error opening procfs\n");
else{
	  /* Opened file successfully */
	  fscanf(file, "%s", line);
	  while (!feof(file)){
		  if (strcmp(line, search_token) == 0){
			 fscanf(file, "%s", line);
			 if (flag == 0){ // Prints to end of line
				fgets(line, MAXLINELENGTH, file);
				}
			 else if(flag == 1) {// Prints next token
				printf(" ");
				fscanf(file, "%s", line);
			 }
			 else if(flag == 2){
				printf(" ");
			 }
			 else if(flag == 3){
				printf(" ");
				fscanf(file, "%s", line);
				fscanf(file, "%s", line);
			 }
			 else if(flag == 4){
				printf(" ");
				fscanf(file, "%s", line);
				fscanf(file, "%s", line);
				fscanf(file, "%s", line);
			 }
			 printf("%s", line);
			 break;
		  }
		  fscanf(file, "%s", line);
	  }
	  fclose(file);
}
return 0;
}
