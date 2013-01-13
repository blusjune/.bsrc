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

#include <errno.h>
#include <inttypes.h>
#include <stdio.h>
#include <stdlib.h>

#include "output.h"
#include "os/functions.h"
#include "seqFormat.h"
#include "uflip_timer.h"

/** 128KB IOs (256 sectors) */
#define BLOCKSIZE (SECTOR * BLOCK)

void
seqFormat (UflipParams *PB)
{
  int         status       = false;
  int64_t     ullFilePos;
  int64_t     new_pos;              /* position at which to do the next IO */
  void       *lpRequest    = NULL;  /* IO buffer */
  UflipTimer *timer;
  UflipTimer *timer2;               /* a timer */
  double      Clock;
  double      Clock2;               /* elapsed clock time (before) */
  double      dummy;                /* elapsed user time (unused) */
  double      pClock       = 0;
  double      pClock2      = 0;     /* elapsed clock time (after) */
  double      timeStep;             /* Time for sequential format */
  int32_t     nbSectors;            /* Number of 512B sectors in the device */
  int32_t     realPos;
  int32_t     step;
  int32_t     step_dbg;
  int32_t     step_dbg_inc = PB->deviceSize;
  char        str [MAX_STR];        /* Temporary string */
  FILE       *dbg = NULL;

  nbSectors = PB->deviceSize;
  nbSectors = ((int32_t) (nbSectors / 256)) * 256; /* Must be a multiple of 256... */

  /* allocate IO request data buffer */
  uflip_malloc_aligned (&lpRequest, BLOCKSIZE); /* 128 KB */
  if (lpRequest == NULL)
    HandleError (__func__, "lpRequest buffer allocation failed", 0, ERR_ABORT);

  memset (lpRequest, 0, BLOCKSIZE);

  status = uflip_device_open (PB->device, NO_SYSTEM_BUFFER | NO_HD_CACHE);
  if (status < 0)
    {
      int err = get_last_error ();
      uflip_free_aligned (lpRequest);
      HandleError (__func__, "Unable to open device", err, ERR_ABORT);
    }

  timer = uflip_timer_new ();
  if (timer == NULL)
    {
      int err = get_last_error ();
      uflip_device_close (PB->device);
      HandleError (__func__, "Unable to allocate timer", err, ERR_ABORT);
    }

  timer2 = uflip_timer_new ();
  if (timer2 == NULL)
    {
      int err = get_last_error ();
      uflip_device_close (PB->device);
      HandleError (__func__, "Unable to allocate timer2", err, ERR_ABORT);
    }

  if (uflip_timer_start (timer) == -1)
    {
      int err = get_last_error ();
      uflip_device_close (PB->device);
      HandleError (__func__, "Unable to start timer", err, ERR_ABORT);
    }

  if (uflip_timer_get_elapsed_time (timer, &pClock, &dummy, &dummy) == -1)
    {
      int err = get_last_error ();
      uflip_device_close (PB->device);
      HandleError (__func__, "Unable to get elapsed time", err, ERR_ABORT);
    }

  OutputString (OUT_TRA, "Starting sequential format of the device\n");

  if (!(PB->fake))
    {
      strcpy (str, "sector;timing;size;\n");
      dbg = fopen ("TRACES\\DEBUG_SEQFORMAT.csv", "w");
      if (fwrite (str, sizeof (char), strlen (str), dbg) < strlen (str))
        HandleError (__func__, "Unable to write debug data", errno, ERR_SUCCESS);

      /* calculate the optimal step to get only 65536 records (Excel can't open beyond).
       = sum([size of the device] / 256) / 65536 */
      step_dbg_inc = PB->deviceSize / (256 * 65536) + 1;
    }

  step     = 1000;
  step_dbg = step_dbg_inc;
  for (realPos = 0; realPos < nbSectors; realPos += 256)
    {
      if ((realPos / 256) > step)
        {
          printf (".");
          step += 1000;
        }

      ullFilePos  = realPos;
      ullFilePos *= SECTOR;
      new_pos = uflip_device_seek (PB->device, ullFilePos, SEEK_SET);
      if (new_pos < 0)
        {
          int err = get_last_error ();
          sprintf (str, "Unable to set file pointer to %"PRId64" (in blocks %"PRId64")", ullFilePos,
                   ullFilePos / (int32_t) BLOCKSIZE);
          uflip_device_close (PB->device);
          HandleError (__func__, str, err, ERR_ABORT);
        }
      ullFilePos = new_pos;

      /*printf ("[IO #%d] Going to write %I32d bytes at byte %I64d (sector %I32d)\n",
              realPos / 256, BLOCKSIZE, ullFilePos, realPos);*/

      /* we don't want to write anything in fake mode */
      if (PB->fake)
        continue;

      /* let's write then */
      if (uflip_timer_start (timer2) == -1)
        {
          int err = get_last_error ();
          uflip_device_close (PB->device);
          HandleError (__func__, "Unable to start timer", err, ERR_ABORT);
        }
      if (uflip_timer_get_elapsed_time (timer2, &pClock2, &dummy, &dummy) == -1)
        {
          int err = get_last_error ();
          uflip_device_close (PB->device);
          HandleError (__func__, "Unable to get elapsed time", err, ERR_ABORT);
        }
      status = uflip_device_write (PB->device, lpRequest, BLOCKSIZE);
      int err = get_last_error ();
      if (uflip_timer_get_elapsed_time (timer2, &Clock2, &dummy, &dummy) == -1)
        {
          int errn = get_last_error ();
          uflip_device_close (PB->device);
          HandleError (__func__, "Unable to get elapsed time", errn, ERR_ABORT);
        }
      timeStep = Clock2 - pClock2;
      /* Tries to exit as gracefully as possible in case of failure */
      if (status == -1 || status == 0)
        {
          sprintf (str, "File write of %"PRId32" bytes failed - file pointer %"PRId64" (in block %"PRId64")",
                   BLOCKSIZE, ullFilePos, ullFilePos / (int32_t) BLOCKSIZE);
          uflip_device_close (PB->device);
          HandleError (__func__, str, err, ERR_ABORT);
        }

      /* log every step_dbg_inc IOs */
      if ((realPos / 256) > step_dbg)
        {
          sprintf (str, "%"PRId32";%f;%"PRId32";\n", realPos, timeStep, 256 * 512);
          if (fwrite (str, sizeof (char), strlen (str), dbg) < strlen (str))
            HandleError(__func__, "Unable to write debug data", errno, ERR_SUCCESS);

          step_dbg += step_dbg_inc;
        }
    }

  /* Kind of a report for the user */
  if (uflip_timer_get_elapsed_time (timer, &Clock, &dummy, &dummy) == -1)
    {
      int err = get_last_error ();
      uflip_device_close (PB->device);
      HandleError (__func__, "Unable to get elapsed time", err, ERR_ABORT);
    }
  timeStep = Clock - pClock;
  sprintf (str,  "End of sequential format\n"
           "\t%u IOs of size 128.0 KB performed\n"
           "\tDuration %f, (avg for each IO: %f)\n",
           realPos / 256, timeStep, timeStep / (realPos / 256));
  OutputString (OUT_TRA, str);

  if (!(PB->fake))
    {
      if (fclose (dbg) == EOF)
        {
          HandleError (__func__, "Unable to close debug file.", errno, ERR_SUCCESS);
        }
      dbg = NULL;
    }

  /* Exits on success */
  uflip_timer_destroy (timer2);
  uflip_timer_destroy (timer);
  uflip_device_close (PB->device);
  uflip_free_aligned (lpRequest);
}
