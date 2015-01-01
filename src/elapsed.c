/*
 * PROJECT: Development of a key-value database (includes client)
 * AUTHORS: Marcos Bjorkelund
 * NOTICE:  THIS WAS CREATED FOR A SUBJECT IN UNIVERSITY OF SEVILLE'S
 *          HIGHER TECHNICAL SCHOOL OF ENGINEERING AS A COURSE PROJECT.
 *          PLEASE DO NOT DISTRIBUTE OR PUBLISH ANY MODIFICATION UNTIL
 *          IT GETS RELEASED PUBLICLY.
 */

#include <stdio.h>
#include <unistd.h>
#include <sys/timeb.h>
#include <sys/times.h>
#include <sys/wait.h>
#include <time.h>

int main(int argc, char *argv[])
{
	struct tms start_tm, end_tm;
	clock_t t_start, t_end;
	long ticks_per_sec;
	struct timeb start, end;

	if(argc < 2)
	{
		fprintf(stderr, "use: %s command [args]\n", argv[0]);
	}
	else
	{
		// to calculate the real elapsed time more precisely
		ftime(&start);

		/* obtiene el número de int. De reloj por segundo*/
		ticks_per_sec = sysconf(_SC_CLK_TCK);
		t_start = times(&start_tm);
   
		if(fork() == 0)
		{
			execvp(argv[1], &argv[1]);
			perror("execvp");
		}
		else
		{
			wait(NULL);
			ftime(&end);
			t_end = times(&end_tm);

			printf("\n---------------\nTIME STATISTICS\n---------------\n");
			printf ("Real elapsed time:\t%.2fs (%lums)\n",
			        (float) (t_end - t_start) / ticks_per_sec,
					(int) 1000.0 * (end.time - start.time)
			              + (end.millitm - start.millitm));
			printf ("User elapsed time:\t%.2fs\n", (float)
			        (end_tm.tms_cutime - start_tm.tms_cutime)/ticks_per_sec);
			printf ("System elapsed time:\t%.2fs\n", (float)
			        (end_tm.tms_cstime - start_tm.tms_cstime)/ticks_per_sec);
			printf("---------------\n");
      }
   }

   return 0; 
}

