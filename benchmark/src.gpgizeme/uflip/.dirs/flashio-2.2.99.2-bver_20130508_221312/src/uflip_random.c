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

 © 1997 - 2007 Agner Fog. All software in this library is published under the
 GNU General Public License with the further restriction that it cannot be
 used for gambling applications. See licence.htm
 © 2008-2010, Luc Bouganim <luc.bouganim@inria.fr>
 © 2009-2010, Lionel Le Folgoc <lionel.le_folgoc@inria.fr>
*/

#include <stdlib.h>

#include "output.h"
#include "uflip_random.h"

UflipRandom*
uflip_random_new (uint32_t seed)
{
  UflipRandom *rnd = NULL;

  rnd = (UflipRandom *) malloc (sizeof (UflipRandom));
  if (rnd != NULL)
    uflip_random_init (rnd, seed);

  return rnd;
}



void
uflip_random_init (UflipRandom *rnd,
                   uint32_t     seed)
{
  int      i;
  uint32_t s = seed;

  if (rnd == NULL)
    {
      /* better to abort than continue with a non-random number */
      HandleError (__func__, "Invalid generator passed!", 0, ERR_ABORT);
    }

  /* make random numbers and put them into the buffer */
  for (i = 0; i < 5; ++i)
    {
      s = s * 29943829 - 1;
      rnd->x [i] = s;
    }

  /* randomize some more */
  for (i = 0; i < 19; ++i)
    uflip_random_get_bits (rnd);
}



int
uflip_random_get_int (UflipRandom *rnd,
                      int          min,
                      int          max)
{
  /* Output random integer in the interval min <= x <= max */
  /* Relative error on frequencies < 2^-32 */
  if (max <= min)
    {
      if (max == min)
        {
          return min;
        }
      else
        {
          return 0x80000000;
        }
    }
  /* Multiply interval with random and truncate */
  int r = (int) ((max - min + 1) * uflip_random_get_dbl (rnd)) + min;
  if (r > max)
    r = max;

  return r;
}



double
uflip_random_get_dbl (UflipRandom *rnd)
{
  return uflip_random_get_bits (rnd) * (1. / (65536. * 65536.));
}



uint32_t
uflip_random_get_bits (UflipRandom *rnd)
{
  uint64_t sum;

  if (rnd == NULL)
    {
      /* better to abort than continue with a non-random number */
      HandleError (__func__, "Invalid generator passed!", 0, ERR_ABORT);
    }

  sum = (uint64_t) 2111111111ul * (uint64_t) rnd->x [3] +
        (uint64_t) 1492 * (uint64_t) (rnd->x [2]) +
        (uint64_t) 1776 * (uint64_t) (rnd->x [1]) +
        (uint64_t) 5115 * (uint64_t) (rnd->x [0]) +
        (uint64_t) rnd->x [4];
  rnd->x [3] = rnd->x [2];
  rnd->x [2] = rnd->x [1];
  rnd->x [1] = rnd->x [0];
  rnd->x [4] = (uint32_t) (sum >> 32); /* Carry */
  rnd->x [0] = (uint32_t) (sum);       /* Low 32 bits of sum */

  return rnd->x [0];
}



void
uflip_random_destroy (UflipRandom *rnd)
{
  free (rnd);
}
