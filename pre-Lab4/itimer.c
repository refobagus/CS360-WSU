/* itimer.c program */

#include <signal.h>
#include <stdio.h>
#include <string.h>
#include <sys/time.h>
#include <time.h>

/*************************
 struct timeval {
    time_t      tv_sec;         // seconds
    suseconds_t tv_usec;        // microseconds
 };
 struct itimerval {
    struct timeval it_interval; // Interval of periodic timer
    struct timeval it_value;    // Time until next expiration
 };
*********************/

int hh, mm, ss;
void timer_handler(int sig);
void timer_handler(int sig) {
  ss = (ss + 1) % 60;
  mm = (mm + !ss) % 60;
  hh = (hh + (!mm && !ss)) % 24;
  printf("%.2d :%.2d : %.2d\r", hh, mm, ss);
  fflush(stdout);
}

int main() {
  struct itimerval timer;
  time_t now;
  struct tm *now_tm;
  now = time(NULL);
  now_tm = localtime(&now);
  ss = now_tm->tm_sec;
  mm = now_tm->tm_min;
  hh = now_tm->tm_hour;

  signal(SIGALRM, &timer_handler);

  /* Configure the timer to expire after 1 sec */
  timer.it_value.tv_sec = 1;
  timer.it_value.tv_usec = 0;

  /* and every 1 sec after that */
  timer.it_interval.tv_sec = 1;
  timer.it_interval.tv_usec = 0;
  printf("\nWALLCLOCK\n");
  setitimer(ITIMER_REAL, &timer, NULL);

  while (1);
}
