/*
 This file is part of the uFLIP software. See www.uflip.org

 uFLIP is free software: you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation, either version 3 of the License, or
 (at your option) any later version.

 uFLIP is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.

 You should have received a copy of the GNU General Public License
 along with uFLIP.  If not, see <http://www.gnu.org/licenses/>.

 uFLIP was initially developed based on SQLIO2 by Leonard Chung although
 almost all SQLIO2 code have disappeared. (leonard@ssl.berkeley.edu - see
 http://research.microsoft.com/en-us/um/siliconvalley/projects/sequentialio/ )
 uFLIP also includes some lines from the pseudo random generator from Agner
 Fog (see http://www.agner.org/random/)

 © 2008-2010, Luc Bouganim <luc.bouganim@inria.fr>
 © 2009-2010, Lionel Le Folgoc <lionel.le_folgoc@inria.fr>
*/

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <stdlib.h>
#include <sys/time.h>
#include <sys/resource.h>

#include "uflip_timer.h"

int
uflip_timer_start (UflipTimer *timer)
{
  struct timeval tv;
  struct rusage  ru;

  if (gettimeofday (&tv, NULL) < 0)
    return -1;

  if (getrusage (RUSAGE_SELF, &ru) < 0)
    return -1;

  /* let's keep usec internally */
  timer->clock_time = 1e6 * tv.tv_sec + tv.tv_usec;
  timer->user_time = 1e6 * ru.ru_utime.tv_sec + ru.ru_utime.tv_usec;
  timer->kernel_time = 1e6 * ru.ru_stime.tv_sec + ru.ru_stime.tv_usec;

  return 0;
}



int
uflip_timer_get_elapsed_time (UflipTimer *timer,
                              double     *clock_time,
                              double     *user_time,
                              double     *kernel_time)
{
  struct timeval tv;
  struct rusage  ru;

  if (gettimeofday(&tv, NULL) < 0)
    return -1;

  if (getrusage(RUSAGE_SELF, &ru) < 0)
    return -1;

  /* caller expects seconds */
  *clock_time = (1e6 * tv.tv_sec + tv.tv_usec - timer->clock_time) / 1e6;
  *user_time = (1e6 * ru.ru_utime.tv_sec + ru.ru_utime.tv_usec - timer->user_time) / 1e6;
  *kernel_time = (1e6 * ru.ru_stime.tv_sec + ru.ru_stime.tv_usec - timer->kernel_time) / 1e6;

  return 0;
}
