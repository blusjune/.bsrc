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

#include <inttypes.h>
#include <math.h>
#include <stdlib.h>
#include <stdio.h>

#include "genBench.h"
#include "microbench.h"
#include "output.h"
#include "os/functions.h"
#include "uflip_random.h"
#include "uflip_timer.h"

extern UflipRandom *rg;

UflipResults
*runBench (UflipParams *PB)
{
  int           cache_flags = 0;  /* file flags for system/hardware caches */
  int64_t       ullFilePos;
  int64_t       new_pos;          /* position for the next IO */
  int           status = false;   /* status from latest call */
  void         *lpRequest = NULL; /* IO buffer */
  UflipResults *RB = NULL;        /* sRB to be returned */
  UflipTimer   *timer;            /* a timer */
  double        fClock;           /* elapsed clock time */
  double        fUser;            /* elapsed user time */
  double        fKernel;          /* elapsed kernel time */
  double        fpClock = 0;      /* elapsed clock time */
  double        fpUser = 0;       /* elapsed user time */
  double        fpKernel = 0;     /* elapsed kernel time */
  char          ErrMsg [MAX_STR]; /* Msg in case of error */
  long          currErase = 0;    /* Current number of erases */
  int32_t       i;                /* Current IO  */
  int32_t       LBA;              /* position in sector for the next IO */
  bool          isSeq;            /* Baseline pattern for normal patterns */
  bool          isRead;           /* Baseline pattern for normal patterns */
  bool          isSeq1;           /* Baseline pattern for mix patterns */
  bool          isRead1;          /* Baseline pattern for mix patterns */
  bool          isSeq2;           /* Baseline pattern for mix patterns */
  bool          isRead2;          /* Baseline pattern for mix patterns */
  bool          doRead;           /* for indicators */
  bool          doSeq;            /* for indicators */
  int32_t       iP1;              /* Current IO for pattern 1 */
  int32_t       iP2;              /* Current IO for pattern 2 */
  int32_t       pattern;          /* Current Pattern (0 (no Mix), 1 or 2) */

  RB = uflip_results_new (); /* allocate a new sResults */
  if (RB == NULL)
    HandleError (__func__, "Couldn't allocate sResults structure", 0, ERR_ABORT);

  RB->success    = false;     /* by default, the test is not successful until we reach the end */
  RB->minIO      = INT32_MAX; /* measured clock time in seconds for the cheapest IO (not in place) */
  RB->maxIO      = 0;         /* measured clock time in seconds for the more expensive IO (not in place) */
  RB->avgIO      = 0;         /* measured clock time in seconds for IO (average)(not in place) */
  RB->timeClock  = 0;
  RB->lostTime   = 0;
  RB->timeKernel = 0;
  RB->timeUser   = 0;
  RB->eraseCount = 0;

  /* Handle the special case of setup experiment where we want to perform Seq Read, then Rnd Writes, then */
  /* Seq Read again with countIOs. */
  /* We thus multiply IOCount by 3 to Count and detect the limit within the loop... */
  if (strcasecmp (PB->comment, "SPE")== 0)
    PB->IOCount *= 3;

  /* allocate detailed measurement Tables */
  RB->timing = (double *) calloc (PB->IOCount, sizeof (double));
  if (RB->timing == NULL)
    HandleError (__func__, "Couldn't allocate timing table", 0, ERR_ABORT);

  RB->dPos = (int *) calloc (PB->IOCount, sizeof (int));
  if (RB->dPos == NULL)
    HandleError (__func__, "Couldn't allocate dPos table", 0, ERR_ABORT);

  RB->isRND = (bool *) calloc (PB->IOCount, sizeof (bool));
  if (RB->isRND == NULL)
    HandleError (__func__, "Couldn't allocate isRND table", 0, ERR_ABORT);

  RB->isWrite = (bool *) calloc (PB->IOCount, sizeof (bool));
  if (RB->isWrite == NULL)
    HandleError (__func__, "Couldn't allocate isWrite table", 0, ERR_ABORT);

  /* allocate IO request data buffer */
  uflip_malloc_aligned (&lpRequest, PB->IOSize * SECTOR);
  if (lpRequest == NULL)
    HandleError (__func__, "lpRequest buffer allocation failed", 0, ERR_ABORT);

  /* Disable (or not) caches */
  cache_flags |= ((PB->bufferType & FS_BUFFERING) ? 0 : NO_SYSTEM_BUFFER);
  cache_flags |= ((PB->bufferType & HW_BUFFERING) ? 0 : NO_HD_CACHE);

  /* Open the device */
  status = uflip_device_open(PB->device, cache_flags);
  if (status < 0)
    {
      int err = get_last_error ();
      HandleError (__func__, "Unable to open the device", err, ERR_ABORT);
    }

  /* -------------------MAIN LOOP Initialization --------------------------------------------------- */
  if (PB->collectErase != INT32_MAX)
    {
      currErase = uflip_device_get_erase_count (PB->device, PB->collectErase);
      RB->initErase = currErase; /* initial nb of erase */
    }
  else
    {
      RB->initErase = 0;
    }
  /* Creates the timers */
  timer = uflip_timer_new ();
  if (timer == NULL)
    {
      int err = get_last_error ();
      uflip_device_close (PB->device);
      HandleError (__func__, "Unable to allocate timer", err, ERR_ABORT);
    }
  /* Starts the timer */
  if (uflip_timer_start (timer) == -1)
    {
      int err = get_last_error ();
      uflip_device_close (PB->device);
      HandleError (__func__, "Unable to start timer", err, ERR_ABORT);
    }
  /* Get the elapsed time */
  if (uflip_timer_get_elapsed_time (timer, &fClock, &fUser, &fKernel) == -1)
    {
      int err = get_last_error ();
      uflip_device_close (PB->device);
      HandleError (__func__, "Unable to get elapsed time", err, ERR_ABORT);
    }

  if (strcasecmp (PB->base, "SR") == 0)
    {
      isSeq = true;
      isRead = true;
      isSeq1 = true;
      isRead1 = true;
    }
  if (strcasecmp (PB->base, "SW") == 0)
    {
      isSeq = true;
      isRead = false;
      isSeq1 = true;
      isRead1 = false;
    }
  if (strcasecmp (PB->base, "RR") == 0)
    {
      isSeq = false;
      isRead = true;
      isSeq1 = false;
      isRead1 = true;
    }
  if (strcasecmp (PB->base, "RW") == 0)
    {
      isSeq = false;
      isRead = false;
      isSeq1 = false;
      isRead1 = false;
    }
  if (strcasecmp (PB->base2, "SR") == 0)
    {
      isSeq2 = true;
      isRead2 = true;
    }
  if (strcasecmp (PB->base2, "SW") == 0)
    {
      isSeq2 = true;
      isRead2 = false;
    }
  if (strcasecmp (PB->base2, "RR") == 0)
    {
      isSeq2 = false;
      isRead2 = true;
    }
  if (strcasecmp (PB->base2, "RW") == 0)
    {
      isSeq2 = false;
      isRead2 = false;
    }

  /* -------------------MAIN LOOP--------------------------------------------------- */
  iP1 = 0;  /* for Mix */
  iP2 = 0;  /* for Mix */
  for (i = 0; i < PB->IOCount; ++i)
    {
      if ((i == PB->IOCount / 3) && (strcasecmp (PB->comment, "SPE") == 0))
        {
          /* Change pattern from Seq Read to Rand Writes */
          isSeq = false;
          isRead = false;
        }
      if ((i == 2 * PB->IOCount / 3) && (strcasecmp (PB->comment, "SPE") == 0))
        {
          /* Change pattern from Rand Writes to Seq Read */
          isSeq = true;
          isRead = true;
        }
      /* if MIX pattern, decide if we are currently doing Pattern1 or Pattern2 */
      if (PB->microBenchID == MIX)
        {
          if (PB->ratio >= 0)
            {
              if ((i % (PB->ratio + 1)) == 0)
                {
                  pattern = 2;
                }
              else
                {
                  pattern = 1;
                }
            }
          if (PB->ratio < 0)
            {
              if ((i % (-PB->ratio + 1)) == 0)
                {
                  pattern = 1;
                }
              else
                {
                  pattern = 2;
                }
            }
        }
      else
        {
          pattern = 0;
        }

      if (pattern == 1)
        {
          if (isSeq1 == true)
            {
              LBA = iP1 * PB->IOSize;
              LBA %= PB->targetSize;
              LBA += PB->targetOffset;
            }
          else
            {
              LBA = PB->targetOffset + uflip_random_get_int (rg, 0, (PB->targetSize / PB->IOSize) - 1) * PB->IOSize;
            }
          doRead = isRead1;
          doSeq = isSeq1;
          ++iP1;
        }
      else if (pattern == 2)
        {
          if (isSeq2 == true)
            {
              LBA = iP2 * PB->IOSize;
              LBA %= PB->targetSize2;
              LBA += PB->targetOffset2;
            }
          else
            {
              LBA = PB->targetOffset2 + uflip_random_get_int (rg, 0, (PB->targetSize2 / PB->IOSize) - 1) * PB->IOSize;
            }
          doRead = isRead2;
          doSeq = isSeq2;
          ++iP2;
        }
      else
        {
          /* pattern = 0 */
          if (isSeq == true)
            {
              if (PB->nbPartition > 1)
                {
                  int32_t PS;
                  int32_t Pi;
                  int32_t Oi;

                  PS = PB->targetSize / PB->nbPartition;
                  Pi = i % PB->nbPartition;
                  Oi = (((int32_t) (i / PB->nbPartition)) * PB->IOSize) % PS;
                  LBA = PS * Pi + Oi + PB->IOShift;
                }
              else
                {
                  LBA = PB->order * i * PB->IOSize + PB->IOShift;
                }
              LBA %= PB->targetSize;
              LBA += PB->targetOffset;
            }
          else
            {
              LBA = PB->targetOffset + uflip_random_get_int (rg, 0, (PB->targetSize / PB->IOSize) - 1) * PB->IOSize + PB->IOShift;
            }
          doRead = isRead;
          doSeq = isSeq;
        }

      if ((LBA < 0) ||(LBA > PB->deviceSize))
        {
          sprintf (ErrMsg, "OUT OF BOUND LBA : LBA %d Dev Size %d TS %d TO %d Shift %d TS2 %d TO2 %d IOSIZE %d \n",
                   LBA, PB->deviceSize, PB->targetSize, PB->targetOffset, PB->IOShift, PB->targetSize2, PB->targetOffset2, PB->IOSize);
          uflip_device_close (PB->device);
          HandleError (__func__, ErrMsg, get_last_error() , ERR_ABORT);
        }

      /* ----------------------------------- COMMON FOR ANY IO */
      ullFilePos  = LBA;
      ullFilePos *= SECTOR;

      /* Set the pointer in file */
      new_pos = uflip_device_seek (PB->device, ullFilePos, SEEK_SET);
      if (new_pos < 0)
        {
          int err = get_last_error ();
          sprintf (ErrMsg, "Unable to set file pointer to %"PRId64" (in blocks %"PRId64")", ullFilePos,
                   ullFilePos / (PB->IOSize * SECTOR));
          uflip_device_close (PB->device);
          HandleError (__func__, ErrMsg, err, ERR_ABORT);
        }
      ullFilePos = new_pos;
      /* get the elapsed time */
      if (uflip_timer_get_elapsed_time (timer, &fpClock, &fpUser, &fpKernel) == -1)
        {
          int err = get_last_error ();
          uflip_device_close (PB->device);
          HandleError (__func__, "Unable to get elapsed time", err, ERR_ABORT);
        }
      RB->lostTime += fpClock - fClock;

      if (doSeq)
        {
          RB->isRND [i] = false;
        }
      else
        {
          RB->isRND [i] = true;
        }

      if (doRead)
        {
          /* READ case */
          RB->isWrite [i] = false;
          /*printf("for iteration %d READING block %"PRId64"\n", i, ullFilePos);*/
          if (PB->fake == false)
            {
              status = uflip_device_read (PB->device, lpRequest, PB->IOSize * SECTOR);
              int err = get_last_error ();
              if (status == -1)
                {
                  sprintf (ErrMsg, "File read failed - file pointer %"PRId64" (in blocks %"PRId64")", ullFilePos,
                           ullFilePos / (PB->IOSize * SECTOR));
                  uflip_device_close (PB->device);
                  HandleError (__func__, ErrMsg, err, ERR_ABORT);
                }
              if (status == 0)
                {
                  sprintf(ErrMsg, "Early EOF received - file pointer %"PRId64" (in blocks %"PRId64")", ullFilePos,
                          ullFilePos / (PB->IOSize * SECTOR));
                  uflip_device_close (PB->device);
                  HandleError (__func__, ErrMsg, err, ERR_ABORT);
                }
            }
        }
      else
        {
          /* WRITE case */
          RB->isWrite [i] = true;
          /*printf("for iteration %d WRITING block %"PRId64" of size %lu\n", i, ullFilePos/SECTOR, PB->IOSize*SECTOR);*/
          if (PB->fake == false)
            {
              status = uflip_device_write (PB->device, lpRequest, PB->IOSize * SECTOR);
              int err = get_last_error ();
              if (status == -1)
                {
                  sprintf (ErrMsg, "File write failed - file pointer %"PRId64" (in blocks %"PRId64")", ullFilePos,
                           ullFilePos/(PB->IOSize*SECTOR));
                  uflip_device_close (PB->device);
                  HandleError (__func__, ErrMsg, err, ERR_ABORT);
                }
              if (status == 0)
                {
                  sprintf (ErrMsg, "Nothing written - file pointer %"PRId64" (in blocks %"PRId64")", ullFilePos,
                           ullFilePos / (PB->IOSize * SECTOR));
                  uflip_device_close (PB->device);
                  HandleError (__func__, ErrMsg, err, ERR_ABORT);
                }
            }
        }
      if (uflip_timer_get_elapsed_time (timer, &fClock, &fUser, &fKernel) == -1)
        {
          int err = get_last_error ();
          uflip_device_close (PB->device);
          HandleError (__func__, "Unable to get elapsed time", err, ERR_ABORT);
        }
        RB->timing [i]  = fClock - fpClock;
        RB->dPos   [i]  = ullFilePos / SECTOR;
        RB->timeClock  += fClock - fpClock;  /* clock time */
        RB->timeKernel += fKernel -fpKernel; /* kernel time */
        RB->timeUser   += fUser - fpUser;    /* user time */
      if (i >= PB->ignoreIO)
        {
          if (fClock - fpClock < RB->minIO)
            RB->minIO = fClock - fpClock;

          if (fClock - fpClock > RB->maxIO)
            RB->maxIO = fClock - fpClock;

          RB->avgIO += fClock - fpClock;
        }
      /* makes the I/O pause */
      if ((PB->pauseIO != 0) && (PB->fake == false))
        {
          if (uflip_wait_microsec (PB->pauseIO) == -1)
            {
              int err = get_last_error ();
              uflip_device_close (PB->device);
              HandleError (__func__, "Unable to add pauses", err, ERR_ABORT);
            }
        }
      if (((i+1) % PB->burstIO == 0) && (PB->pauseBurst != 0) && (PB->fake == false))
        {
          /*printf ("%lu pause\n", i);*/
          if (uflip_wait_microsec (PB->pauseBurst) == -1)
            {
              int err = get_last_error ();
              uflip_device_close (PB->device);
              HandleError (__func__, "Unable to add burst pauses", err, ERR_ABORT);
            }
        }
    }
  if (PB->collectErase != INT32_MAX)
    {
      int32_t x;

      x = currErase;
      currErase = uflip_device_get_erase_count (PB->device, PB->collectErase);
      RB->eraseCount += currErase -x;
      RB->endErase = currErase; /* final nb of erase */
    }
  else
  {
    RB->endErase = 0;
  }

  RB->avgIO /= (float) (PB->IOCount - PB->ignoreIO);

  /* Computing some stats */
  RB->stdDevIO = 0;
  for (int32_t it = PB->ignoreIO; it < PB->IOCount; ++it)
    {
      RB->stdDevIO += (RB->timing [it] - RB->avgIO) * (RB->timing [it] - RB->avgIO);
    }
  RB->stdDevIO /= (float) (PB->IOCount - PB->ignoreIO);
  RB->stdDevIO = sqrt (RB->stdDevIO);
  printf("Erases [%d],  ", RB->eraseCount);
  printf("AvgIO [%d]\n ", (int32_t) (RB->avgIO * 1000000.0));

  RB->success = true;  /* success! */
  /* ------------------------------------------------------------------------------- */
  /* Common Exit, close handles, deallocate storage */
  uflip_timer_destroy (timer);
  uflip_device_close (PB->device);
  uflip_free_aligned (lpRequest);
  /* make the final pause */
  if ((PB->pauseExp != 0)  && (PB->fake == false))
    {
      if (uflip_wait_microsec (PB->pauseExp) == -1)
        HandleError (__func__, "Unable to add final pause", get_last_error(), ERR_ABORT);
    }

  return RB;
}
