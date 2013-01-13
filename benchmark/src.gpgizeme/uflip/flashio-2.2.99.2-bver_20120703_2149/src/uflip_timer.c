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

#include <stdlib.h>

#include "uflip_timer.h"

int
uflip_wait_microsec (int microseconds)
{
  UflipTimer *timer = NULL;
  double      fClock;
  double      fpClock;
  double      fUser;
  double      fKernel;

  timer = uflip_timer_new ();
  if (timer == NULL)
    return -1;

  /* Start the timer */
  if (uflip_timer_start (timer) == -1)
    {
      uflip_timer_destroy (timer);
      return -1;
    }
  /* Get the elapsed time */
  if (uflip_timer_get_elapsed_time (timer, &fClock, &fUser, &fKernel) == -1)
    return -1;

  while ((fpClock - fClock) * 1000000.0 < (float) microseconds - 8)
    {
      if (uflip_timer_get_elapsed_time (timer, &fpClock, &fUser, &fKernel) == -1)
        return -1;
    }
  uflip_timer_destroy (timer);

  return 0;
}



UflipTimer
*uflip_timer_new (void)
{
  return ((UflipTimer *) malloc (sizeof (UflipTimer)));
}



void
uflip_timer_destroy (UflipTimer *timer)
{
  free (timer);
}
