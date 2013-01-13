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

#include <errno.h>
#include <inttypes.h>
#include <stdio.h>
#include <stdlib.h>

#include "output.h"
#include "os/functions.h"
#include "rndFormat.h"
#include "uflip_random.h"
#include "uflip_timer.h"

extern UflipRandom *rg;

static int min (int a,
                int b);

void
rndFormat (UflipParams *PB)
{
  int64_t     ullFilePos;
  int64_t     new_pos;              /* position at which to do the next IO */
  int         status       = false; /* status from latest call */
  void*       lpRequest    = NULL;  /* IO buffer */
  UflipTimer *timer;
  UflipTimer *timer2;               /* a timer */
  double      Clock;
  double      Clock2;               /* elapsed clock time */
  double      dummy;                /* elapsed user time */
  double      pClock       = 0;
  double      pClock2      = 0;     /* elapsed clock time */
  double      timeStep;             /* Time for one step during format */
  int32_t     nbIO;                 /* Total number of IO to do in blocks */
  int32_t     nbPages;              /* Number of pags of 512 B in the device */
  size_t      blockSize;            /* Varying size of a block */
  int32_t     ioPos;
  int32_t     nbIODone;
  int32_t     IOStep;
  int64_t     realPos;
  int32_t     step;
  int32_t     step_dbg;
  int32_t     step_dbg_inc = INT32_MAX;
  char        str [MAX_STR];        /* Temporary string */
  int32_t    *doneIO;
  FILE       *dbg          = NULL;
  int         errn;

  nbPages = PB->deviceSize;
  nbPages = ((int32_t) (nbPages / 256)) * 256; /* Must be a multiple of 256... */

  /* allocate and initialize the "doneIO" table of long */
  doneIO = (int32_t *) calloc (nbPages, sizeof (int32_t));
  if (doneIO == NULL)
    HandleError (__func__, "doneIO buffer allocation failed", 0, ERR_ABORT);

  for (int32_t k = 0; k < nbPages; ++k)
    doneIO [k] = k;

  /* allocate IO request data buffer */
  uflip_malloc_aligned (&lpRequest, 128 * 1024); /* 128 KB for larger blocks */
  if (lpRequest == NULL)
    HandleError (__func__, "lpRequest buffer allocation failed", 0, ERR_ABORT);

  /* ---------------------------------------------------------------------------------------------- */
  /* Opening the device */
  status = uflip_device_open (PB->device, NO_SYSTEM_BUFFER | NO_HD_CACHE);
  if (status < 0)
    {
      int err = get_last_error ();
      HandleError (__func__, "Unable to open device", err, ERR_ABORT);
    }

  /* -------------------MAIN LOOP Initialization --------------------------------------------------- */
  nbIO = nbPages;
  /* Creates the timers */
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
  /* Starts the timer */
  if (uflip_timer_start (timer) == -1)
    {
      int err = get_last_error ();
      uflip_device_close (PB->device);
      HandleError (__func__, "Unable to start timer", err, ERR_ABORT);
    }
  /* Get the elapsed time */
  if (uflip_timer_get_elapsed_time (timer, &pClock, &dummy, &dummy) == -1)
    {
      int err = get_last_error ();
      uflip_device_close (PB->device);
      HandleError (__func__, "Unable to get elapsed time", err, ERR_ABORT);
    }
  OutputString (OUT_TRA, "Starting formating the device\n");
  int formSize = 256; /* from 128KB (256 sectors) to 512 B (1 sector) */
  blockSize = formSize*512;
  memset (lpRequest, formSize, blockSize);
  int nbShrink = 9; /* 9 steps to go from 128KB to 512B */

  /* -------------------  MAIN LOOP ------------------------------------------------------------------ */

  if (!(PB->fake))
    {
      /* calculate the optimal step to get only 65536 records (Excel can't open beyond).
         = (sum([size of a "shrink"] / [formSize])) / 65536 */
      int64_t tmp = (((int64_t) PB->deviceSize * 511) / (9 * 256 * 65536)) + 1;
      step_dbg_inc = min (tmp, INT32_MAX);
      strcpy (str, "sector;timing;size;\n");
      dbg = fopen ("TRACES"DIR_SEPARATOR"DEBUG_RNDFORMAT.csv", "w");

      if (fwrite (str, sizeof (char), strlen (str), dbg) < strlen (str))
        HandleError (__func__, "Unable to write debug data", errno, ERR_SUCCESS);
    }

  step     = 1000;
  step_dbg = step_dbg_inc;
  IOStep   = 0;
  printf("\nFormating with blocks of size %d \n", formSize * 512);
  for (nbIODone = 0; nbIODone < nbIO; ++nbIODone)
    {
      if (nbIODone > step)
        {
          printf (".");
          /* if STDOUT is buffered, nothing is displayed, so force it */
          fflush (stdout);
          step += 1000;
        }

      if ((nbIO - nbIODone) <= (nbShrink - 1) * (nbIO / 9))
        {
          if (nbShrink != 1)
            {
              /* Get the elapsed time */
              if (uflip_timer_get_elapsed_time (timer, &Clock, &dummy, &dummy) == -1)
                {
                  int err = get_last_error ();
                  uflip_device_close (PB->device);
                  HandleError (__func__, "Unable to get elapsed time", err, ERR_ABORT);
                }
              timeStep = Clock - pClock;
              sprintf (str, "End of phase %d \n", 10 - nbShrink);
              OutputString (OUT_TRA, str);
              sprintf (str, "    %u IOs of size %3.1f KB performed\n", (nbIODone - IOStep) / formSize, (double) formSize / 2);
              OutputString (OUT_TRA, str);
              sprintf (str, "    Duration %f, (avg for each IO: %f)\n", timeStep, timeStep / ((nbIODone - IOStep) / formSize));
              OutputString (OUT_TRA, str);
              IOStep = nbIODone;
              pClock = Clock;
              formSize /= 2;
              --nbShrink;
              blockSize= formSize * 512;
              memset (lpRequest, formSize, blockSize);
              step_dbg = step_dbg_inc;
            }
          else
            {
              printf ("Strange ..... %u\n", nbIODone);
            }
          printf ("\nFormating with blocks of size %d \n", formSize*512);
        }

      ioPos   = uflip_random_get_int (rg, 0,(nbIO - nbIODone) / formSize - 1) * formSize;
      realPos = doneIO [ioPos];
      for (int32_t ll = 0; ll < formSize; ++ll)
        {
          if (doneIO [ioPos + ll] == realPos + ll)
            {
              doneIO [ioPos + ll] = doneIO [nbIO - nbIODone - formSize + ll + ll];
              ++nbIODone;
            }
          else
            {
              sprintf (str, "PBM2 IN FORMAT !!!! %d %d %"PRId64" \n", ll, doneIO [ioPos + ll], realPos + ll);
              uflip_device_close (PB->device);
              HandleError (__func__, str, 0, ERR_ABORT);
            }
        }

      --nbIODone;
      ullFilePos = (int64_t) 512 * realPos;  /* beware the overflow! */
      /* Set the pointer in file */
      new_pos = uflip_device_seek (PB->device, ullFilePos, SEEK_SET);
      if (new_pos < 0)
        {
          int err = get_last_error ();
          sprintf(str, "Unable to set file pointer to %"PRId64" (in blocks %"PRId64")", ullFilePos,
                  ullFilePos / (int32_t) blockSize);
          uflip_device_close (PB->device);
          HandleError (__func__, str, err, ERR_ABORT);
        }
      ullFilePos = new_pos;

      if (!(PB->fake))
        {
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
          status = uflip_device_write (PB->device, lpRequest, blockSize);
          errn = get_last_error ();
          if (uflip_timer_get_elapsed_time (timer2, &Clock2, &dummy, &dummy) == -1)
            {
              int err = get_last_error ();
              uflip_device_close (PB->device);
              HandleError (__func__, "Unable to get elapsed time", err, ERR_ABORT);
            }
          timeStep = Clock2 - pClock2;

          /* log every step_dbg_inc IOs */
          if ((nbIODone - IOStep) / formSize > step_dbg)
            {
              sprintf (str, "%"PRId64";%f;%zu;\n", realPos, timeStep, blockSize);
              if (fwrite (str, sizeof (char), strlen (str), dbg) < strlen (str))
                HandleError(__func__, "Unable to write debug data", errno, ERR_SUCCESS);

              step_dbg += step_dbg_inc;
            }
        }
      else
        {
          status = 1; /* this is fake, so we don't want it to fail these tests. */
        }

      if (status == -1)
        {
          sprintf (str, "File write failed - file pointer %"PRId64" (in blocks %"PRId64")", ullFilePos,
                   ullFilePos / (int32_t) blockSize);
          uflip_device_close (PB->device);
          HandleError (__func__, str, errn, ERR_ABORT);
        }
      if (status == 0)
        {
          sprintf (str, "Nothing written - file pointer %"PRId64" (in blocks %"PRId64")", ullFilePos,
                   ullFilePos / (PB->IOSize * SECTOR));
          uflip_device_close (PB->device);
          HandleError (__func__, str, errn, ERR_ABORT);
        }
    }

  /* Get the elapsed time */
  if (uflip_timer_get_elapsed_time (timer, &Clock, &dummy, &dummy) == -1)
    {
      int err = get_last_error ();
      uflip_device_close (PB->device);
      HandleError (__func__, "Unable to get elapsed time", err, ERR_ABORT);
    }

  timeStep = Clock - pClock;
  sprintf (str, "End of phase %d \n", 10-nbShrink);
  OutputString (OUT_TRA, str);
  sprintf (str, "    %u IOs of size %3.1f KB performed\n", (nbIODone - IOStep) / formSize, (double) formSize / 2);
  OutputString (OUT_TRA, str);
  sprintf (str, "    Duration %f, (avg for each IO: %f)\n", timeStep, timeStep / ((nbIODone - IOStep) / formSize));
  OutputString (OUT_TRA, str);

  if (!(PB->fake))
    {
      if (fclose (dbg) == EOF)
        HandleError (__func__, "Unable to close debug file.", errno, ERR_SUCCESS);

      dbg = NULL;
    }

  /* ------------------------------------------------------------------------------- */
  /* Common Exit, close handles, deallocate storage */
  uflip_timer_destroy (timer2);
  uflip_timer_destroy (timer);
  uflip_device_close (PB->device);
  uflip_free_aligned (lpRequest);
  free (doneIO);
}



static int
min (int a,
     int b)
{
  return a < b ? a : b;
}
