/* loop.c - Measure the time spend in some computations.

   Copyright (c) 2016, Monaco F. J. - <moanco@icmc.usp.br>

   This file is part of Real-Time Sand Box - RTSB

   RTSB is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.
   
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.
   
   You should have received a copy of the GNU General Public License
   along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

/* 

      Can you optimize this code?

*/


#include <stdio.h>
#include <math.h>
#include "utils.h"
#include <stdlib.h>
#include <unistd.h>
#include <time.h>
#include <sys/time.h>
#include <signal.h>
#include <pthread.h>
#include <ncurses.h>

#include "../lib/asciiplay.h"

#include <omp.h>

#define SCHEDULER_POLICY SCHED_RR 
#define SCHEDULER_PRIORITY 99


#define MAX_OUTER_LOOP 100
#define MAX_INNER_LOOP 1000
#define PI 3.1416
#define LENGTH 100

/* This function returns length*cos(angle). */

float foo (int length, int angle)
{
  float b;
  b = length * cos (angle * PI / 180.0);
  return b;
}

/* Main program. */

int main ()
{
  int angle;
  int i, j;
  struct timeval now, before, elapsed;
  double average_loop_time, min_loop_time, max_loop_time;

  average_loop_time = 0;
  min_loop_time = average_loop_time;
  max_loop_time = average_loop_time;
  
  /* Outer loop, to compute averages. */

  select_scheduler (SCHEDULER_POLICY, SCHEDULER_PRIORITY);

  float *values[3];


  for(i = 0; i < 3; ++i) {
    values[i] = (float *) malloc(sizeof(float) * MAX_INNER_LOOP);
  }

  omp_set_num_threads(3);

  #pragma parallel omp private(j, i) shared(values) 
  {
    j = omp_get_thread_num();
    for(i = 0; i < MAX_INNER_LOOP; ++i) {
      angle = (i * 30) % 360;	/* 0, 30, 60, 90 ... 330, 0, 30 ... */
      values[j][i] = foo (LENGTH, angle);
    }
  }

  for(i = 0; i < MAX_INNER_LOOP; ++i) {
    if(fabs(values[0] - values[1]) > 1e-11) values[0] = values[2];
  }

  for (i = 0; i < MAX_OUTER_LOOP; i++) {
      /* Get current time before computation work. */
      
      gettimeofday (&before, NULL);

      
      /*for (j=0; j<MAX_INNER_LOOP; j++) {
        angle = (j * 30) % 360;
        val = foo (LENGTH, angle);
      
        #if DEBUG
          fprintf (stdout, "(%lu) %3d %8.3f %u\n",
          i, angle, val, (unsigned int) elapsed.tv_usec);
        #endif
	    }*/

      /*DO SOMETHING WITH values[i]*/

      
      gettimeofday (&now, NULL);
      timeval_subtract (&elapsed, &now, &before);


      /* Do some statistics. */
      
      average_loop_time += elapsed.tv_usec;

      if (i==0)
	      min_loop_time = elapsed.tv_usec;
      else if (elapsed.tv_usec < min_loop_time)
	      min_loop_time = elapsed.tv_usec;
      
      if (elapsed.tv_usec > max_loop_time)
	      max_loop_time = elapsed.tv_usec;
   
  }

  /* Show statistics. */
  
  average_loop_time = average_loop_time / MAX_OUTER_LOOP;

  printf ("Average loop time: %u\n", (int) average_loop_time);
  printf ("Minimum loop time: %u\n", (int) min_loop_time);
  printf ("Maximum loop time: %u\n", (int) max_loop_time);
  free(values);   
  return 0;
}
