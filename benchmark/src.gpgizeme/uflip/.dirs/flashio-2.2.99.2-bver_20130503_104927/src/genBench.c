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
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "blocAlloc.h"
#include "genBench.h"
#include "input.h"
#include "output.h"
#include "uflip_random.h"

/** Warning, no more memory available on the device (the test won't be exhaustive) */
#define DEVICE_TOO_SMALL 1
/** Error, no more memory available on the device (the test will fail) */
#define TEST_EXCEED_DEVICE 2
/** maximum number of micro-benchs */
#define MAXBENCH 10
/** maximum number of modes per micro-bench */
#define MAXMODE 6
/** Maximum size of a parameter string in Exp Plan */
#define MAX_SIZE_PARAM 30

extern UflipRandom *rg;

static int     max           (int          a,
                              int          b);
static void    GenExp        (FILE        *fp,
                              UflipParams *PB);
static void    ComputeParams (UflipParams *PB,
                              item        *memList,
                              int32_t      value,
                              int32_t      nbVal);
static int     parseSel      (UflipParams *PB,
                              bool        *tabSel);
static int32_t parseExp      (FILE        *fp,
                              int32_t      currExp,
                              UflipParams *PB,
                              int32_t     *tabVal);

/**
 * Generates an experiment (i.e. one run of a micro-bench) using given
 * parameters and writes it to the given file.
 *
 * @param fp output file
 * @param PB a pointer to a structure containing benchmark parameters for this session
 */
static void
GenExp (FILE        *fp,
        UflipParams *PB)
{
  char cm [MAX_STR];

  if (PB->processID > 0)
    {
      sprintf (cm, PAR_PREFIX" "EXEC_PREFIX""PACKAGE_NAME" Run ");
    }
  else
    {
      sprintf (cm, EXEC_PREFIX""PACKAGE_NAME" Run ");
    }

  if (PB->warning != 0)
    sprintf (cm + strlen (cm), "Warn %d ", PB->warning);

  sprintf (cm + strlen (cm), "Com %s ",   PB->comment);
  sprintf (cm + strlen (cm), "Bench %d ", PB->microBenchID);
  sprintf (cm + strlen (cm), "Exp %d ",   PB->expID);
  sprintf (cm + strlen (cm), "Key %d ",   PB->key);
  sprintf (cm + strlen (cm), "Run %d ",   PB->runID);
  sprintf (cm + strlen (cm), "Dev %s ",   uflip_device_to_string (PB->device));
  sprintf (cm + strlen (cm), "IOS %d ",   PB->IOSize);

  switch (PB->microBenchID)
    {
      case ALI:
        sprintf (cm + strlen (cm), "Shift %d ", PB->IOShift);
        break;
      case LOC:
        sprintf (cm + strlen (cm), "TSize %d ", PB->targetSize);
        break;
      case PAT:
        sprintf (cm + strlen (cm), "Part %d ", PB->nbPartition);
        break;
      case ORD:
        sprintf (cm + strlen (cm), "Order %d ", PB->order);
        break;
      case PAR:
        sprintf (cm + strlen (cm), "ParDeg %d PID %d ", PB->parDeg, PB->processID);
        break;
      case MIX:
        sprintf (cm + strlen (cm), "Base2 %s Ratio %d TSize2 %d TOffs2 %d ",
                 PB->base2, PB->ratio, PB->targetSize2, PB->targetOffset2);
        break;
      case PIO:
        sprintf (cm + strlen (cm), "PIO %d ", PB->pauseIO);
        break;
      case PBU:
        sprintf (cm + strlen (cm), "PBurst %d BurstIO %d ",  PB->pauseBurst, PB->burstIO);
        break;
      default:
        /* nothing to do */
        break;
    }

  sprintf (cm + strlen (cm), "Base %s ", PB->base);
  sprintf (cm + strlen (cm), "IOC %d ", PB->IOCount);

  if (PB->microBenchID != LOC)
    sprintf (cm + strlen (cm), "TSize %d ", PB->targetSize);

  sprintf (cm + strlen (cm), "TOffs %d ", PB->targetOffset);
  sprintf (cm + strlen (cm), "IgnIO %d ", PB->ignoreIO);
  sprintf (cm + strlen (cm), "Pause %d ", PB->pauseExp);

  if (PB->collectErase != INT32_MAX)
    sprintf (cm + strlen (cm), "CErase %d ", PB->collectErase);

  if (PB->fake == true)
    strcat (cm, "Fake True ");

  if (PB->bufferType == HW_BUFFERING)
    {
      strcat (cm, "BufferType H ");
    }
  else if (PB->bufferType == FS_BUFFERING)
    {
      strcat (cm, "BufferType S ");
    }
  else if (PB->bufferType == (HW_BUFFERING | FS_BUFFERING))
    {
      strcat (cm, "BufferType A ");
    }

  if (PB->processID > 0)
    sprintf (cm + strlen (cm), " "PAR_SUFFIX);

  strcat (cm, "\n");
  fprintf (fp, "%s", cm);
  OutputString (OUT_LOG, cm);
}



/**
 * @brief Computes Params
 *
 * The difficulty here is to acomodate all the benchmark runs into the memory available
 * on the device without having to "reformat" it.\n
 * Allocation is based on the assumption that benchmark runs are done in a given order:
 * -# read only benchmark (do not modify the "state" of the flash (Exp. 1 to 15)
 * -# random writes (on the whole device) ==> modify slightly, but randomly the state (Exp. 16-22)
 * -# Sequential writes  on focused areas ==> modify the state, but the focus moves\n
 *     ==> the memory is "consumed" sequentially. If no more memory is available warning
 *       DEVICE_TOO_SMALL is set and the offset is chosen randomly (thus the test is not ok)
 * -# Mix patterns SR/SW, RR/SW, SW/RW
 * -# parallelism and partitionned patterns (Exp. 32 & 33)==> Memory is allocated in the
 *     remaining part of the device. Tuning of IOCount and experiments should be done to fit
 *     in the device.
 * -# Ordered patterns (Exp. 34): when the experiment is "focused" (small gaps), we do as
 *     with sequential up to the point where there is no place. Then, it is random.
 *
 * Memory allocation is done thanks to the blocAlloc module.
 *
 * @param PB a pointer to a structure containing benchmark parameters for this session
 * @param memList a pointer to a list of free areas on the device
 * @param value the value of the parameter used for the generation
 * @param nbVal number of values
 */
static void
ComputeParams (UflipParams *PB,
               item        *memList,
               int32_t      value,
               int32_t      nbVal)
{
  static int32_t startAddress = 0;
  static int32_t shift        = 0;
  int32_t        size;
  bool           isRead       = false;
  bool           isSeq        = false;

  printf ("Generates %d.%d with param %d\n", PB->microBenchID, PB->expID,  value);

  if (strcasecmp (PB->base, "SR") == 0)
    {
      PB->ignoreIO = PB->ignoreIOSR;
      PB->IOCount  = PB->IOCountSR;
      isSeq        = true;
      isRead       = true;
    }
  else if (strcasecmp (PB->base, "SW") == 0)
    {
      PB->ignoreIO = PB->ignoreIORR;
      PB->IOCount  = PB->IOCountRR;
      isSeq        = true;
      isRead       = false;
    }
  else if (strcasecmp (PB->base, "RR") == 0)
    {
      PB->ignoreIO = PB->ignoreIOSW;
      PB->IOCount  = PB->IOCountSW;
      isSeq        = false;
      isRead       = true;
    }
  else if (strcasecmp (PB->base, "RW") == 0)
    {
      PB->ignoreIO = PB->ignoreIORW;
      PB->IOCount  = PB->IOCountRW;
      isSeq        = false;
      isRead       = false;
    }

  PB->targetSize = PB->deviceSize; /* Default value for target size */

  switch (PB->microBenchID)
    {
      case GRA:
        PB->IOSize = value;
        break;
       case ALI:
        PB->IOShift = value;
        break;
      case LOC:
        PB->targetSize = value * PB->IOSize;
        break;
      case PAT:
        PB->nbPartition = value;
        break;
      case ORD:
        PB->order = value;
        break;
      case PAR:
        PB->parDeg = value;
        break;
      case MIX:
        PB->ratio = value;
        break;
      case PIO:
        PB->pauseIO = value;
        break;
      case PBU:
        PB->burstIO = value;
        break;
      default:
        /* nothing to do */
        break;
    }

  /* Number of sectors potentially touched by the experiment */
  size = PB->IOSize * PB->IOCount;

  if (PB->IOShift != 0)
    size += PB->IOSize;

  switch (PB->microBenchID)
    {
      case LOC:
        /* 3-LOCALITY */
        size = PB->targetSize;
        if ((isRead == true) || (value > 1024))
          {
            PB->targetOffset = uflip_random_get_int (rg, 0, (int32_t) ((PB->deviceSize - size) / BLOCK)) * BLOCK;
          }
        else
          {
            PB->targetOffset = MemSearch (memList, size);
            /*printf ("==>%d  %d\n",size, PB->targetOffset);*/
          }
        break;
      case ORD:
        /* 5-ORDER */
        size = PB->IOSize * abs (PB->order) * PB->IOCount; /*XXX: wtf? */
        size = PB->IOSize * PB->IOCount * PB->order;
        if (size == 0)
          size = BLOCK;

        PB->targetSize = abs (size);
        if (isRead == true)
          {
            PB->targetOffset =  uflip_random_get_int (rg, 0, (int32_t) ((PB->deviceSize - PB->targetSize) / BLOCK)) * BLOCK;
            if (size < 0)
              PB->targetOffset =  PB->deviceSize - PB->targetOffset;
          }
        else
          {
            PB->targetOffset = MemSearch (memList, abs(size));
            if (PB->targetOffset == INT32_MAX)
              {
                PB->targetOffset = uflip_random_get_int (rg, 0, (int32_t) ((PB->deviceSize - abs (size))/BLOCK)) * BLOCK;
                PB->warning = DEVICE_TOO_SMALL;
                OutputString (OUT_LOG, "DEVICE TOO SMALL \n");
              }
            if (size < 0)
              PB->targetOffset =  PB->targetOffset - size;
          }
        break;
      case GRA:
      case ALI:
      case PIO:
      case PBU:
        /* 1-GRANULARITY , 2-ALIGNMENT, 8-PAUSE, 9-BURST */
        if (isSeq == false)
          {
            PB->targetOffset = 0;
          }
        else if (isRead == true)
          {
            PB->targetOffset = uflip_random_get_int (rg, 0, (int32_t) ((PB->targetSize - size) / BLOCK)) * BLOCK;
            PB->targetSize   = size;
          }
        else
          {
            PB->targetOffset = MemSearch (memList, size);
            PB->targetSize   = size;
          }
        break;
      case MIX:
        /* 07-MIX */
        {
          int32_t I1 = PB->ignoreIO;
          int32_t I2 = 0;
          int32_t C1 = PB->IOCount;
          int32_t C2 = 0;
          int32_t size1;
          int32_t size2;

          if (strcasecmp (PB->base2, "SR") == 0)
            {
              I2 = PB->ignoreIOSR;
              C2 = PB->IOCountSR;
            }
          else if (strcasecmp (PB->base2, "SW") == 0)
            {
              I2 = PB->ignoreIOSW;
              C2 = PB->IOCountSW;
            }
          else if (strcasecmp (PB->base2, "RR") == 0)
            {
              I2 = PB->ignoreIORR;
              C2 = PB->IOCountRR;
            }
          else if (strcasecmp (PB->base2, "RW") == 0)
            {
              I2 = PB->ignoreIORW;
              C2 = PB->IOCountRW;
            }

          if (PB->ratio < 0)
            {
              PB->ignoreIO = max (I1 + (I1 - 1) * (-PB->ratio) + 1, I2 * (-PB->ratio + 1) / (-PB->ratio) + 1);
              PB->IOCount  = max (C1 - I1, C2 - I2) + PB->ignoreIO;
              size1        = PB->IOSize * (1 + (int32_t) (PB->IOCount / (-PB->ratio + 1)));
              size2        = PB->IOSize * (1 + (int32_t) ((PB->IOCount / (-PB->ratio + 1)) * (-PB->ratio)));
            }
          else if (PB->ratio > 0)
            {
              PB->ignoreIO = max (I2 + (I2 - 1) * PB->ratio + 1, (I1 * (PB->ratio + 1) / PB->ratio) + 1);
              PB->IOCount  = max (C1 - I1, C2 - I2) + PB->ignoreIO;
              size1        = PB->IOSize * (1 + (int32_t)(PB->IOCount / (PB->ratio + 1)));
              size2        = PB->IOSize * (1 + (int32_t)((PB->IOCount / (PB->ratio + 1)) * (PB->ratio)));
            }
          else
            {
              PB->ignoreIO = I2;
              PB->IOCount = C2;
              size1 = 0;
              size2 = PB->IOSize * PB->IOCount;
            }

          if (((strcasecmp (PB->base, "SR") == 0) && (strcasecmp (PB->base2, "RR") == 0)) ||
              ((strcasecmp (PB->base, "SR") == 0) && (strcasecmp (PB->base2, "RW") == 0)))
            {
              PB->targetSize    = size1;
              PB->targetOffset  = uflip_random_get_int (rg, 0, (int32_t) ((PB->deviceSize - size1) / BLOCK)) * BLOCK;
              PB->targetOffset2 = 0;
              PB->targetSize2   = PB->deviceSize;
            }
          else if ((strcasecmp (PB->base, "RR") == 0) && (strcasecmp (PB->base2, "RW") == 0))
            {
              PB->targetSize    = PB->deviceSize;
              PB->targetOffset  = 0;
              PB->targetOffset2 = 0;
              PB->targetSize2   = PB->deviceSize;
            }
          else if ((strcasecmp (PB->base, "RR") == 0) && (strcasecmp (PB->base2, "SW") == 0))
            {
              PB->targetOffset2 = MemSearch (memList, size2);
              PB->targetSize2   = size2;
              PB->targetOffset  = MemMinAddress (memList);
              PB->targetSize    = PB->deviceSize - PB->targetOffset;
            }
          else if ((strcasecmp (PB->base, "SW") == 0) &&  (strcasecmp (PB->base2, "RW") == 0))
            {
              PB->targetOffset  = MemSearch (memList, size1);
              PB->targetSize    = size1;
              PB->targetOffset2 = MemMinAddress (memList);
              PB->targetSize2   = PB->deviceSize - PB->targetOffset2;
            }
          else if ((strcasecmp (PB->base, "SR") == 0) && (strcasecmp (PB->base2, "SW") == 0))
            {
              PB->targetOffset2 = MemSearch (memList, size2);
              PB->targetSize2 = size2;
              PB->targetOffset = uflip_random_get_int (rg, (int32_t) ((PB->deviceSize - MemMinAddress (memList)) / 2) / BLOCK, (int32_t) ((PB->deviceSize - size1) / BLOCK)) * BLOCK;
              PB->targetSize = size1;
            }
          }
        break;
      case PAR:
        /* 6-PARALLELISM */
        PB->targetSize = ((int32_t) ((PB->deviceSize / PB->parDeg) / BLOCK)) * BLOCK;
        if (isSeq == false)
          {
            PB->targetOffset =  PB->processID * PB->targetSize;
          }
        else if (isRead == true)
          {
            PB->targetOffset =  uflip_random_get_int(rg, (PB->processID * PB->targetSize)/BLOCK,
                                                     ((PB->processID + 1) * PB->targetSize - size)/BLOCK) * BLOCK;
            PB->targetSize = size;
          }
        else
          {
            /* SEQUENTIAL WRITE..... CAUTION : PARALLEL EXPERIMENT MUST BE AFTER PARTITIONING */
            PB->targetSize = (PB->deviceSize - startAddress) / PB->parDeg;

            if (PB->targetSize < size)
              HandleError (__func__, "device too small", 0, ERR_ABORT);

            PB->targetOffset = MemAllocNearestAfterA (memList, (PB->processID) * PB->targetSize + startAddress, size);
            if (PB->targetOffset == -1)
              HandleError (__func__, "Allocation problem!", 0, ERR_ABORT);

            PB->targetSize = size;
          }
        break;
      case PAT:
        /* 4-PARTITIONING */
        if ((isRead == true) || (PB->nbPartition > 16))
          {
            PB->targetOffset =  0;
            PB->targetSize = ((int32_t) (PB->deviceSize / (MAX_PARTITIONS * BLOCK))) * MAX_PARTITIONS * BLOCK;
          }
        else
          {
            /* SEQUENTIAL WRITE..... CAUTION : PARTITIONNING EXPERIMENT MUST BE THE FIRST ... */
            if (startAddress == 0)
              startAddress = MemMinAddress(memList);

            if ((size % (16 * BLOCK)) != 0)
              size = ((int32_t) (size / (16 * BLOCK)) + 1 ) * (16 * BLOCK);

            PB->targetSize = PB->deviceSize - startAddress - size * nbVal * PB->nbRun;
            PB->targetSize = (int32_t) (PB->targetSize / (16 * BLOCK)) * (16 * BLOCK);
            if (PB->targetSize < size)
              HandleError (__func__, "device too small", 0, ERR_ABORT);

            PB->targetOffset = startAddress + shift;
            for (int32_t k = 0; k < PB->nbPartition; ++k)
              {
                long temp;
                temp = MemAlloc(memList, PB->targetOffset + k * PB->targetSize / PB->nbPartition,
                                PB->targetOffset + k * PB->targetSize / PB->nbPartition + size / PB->nbPartition);
                if (temp == -1)
                  HandleError (__func__, "Allocation problem!", 0, ERR_ABORT);

              }
            shift = shift + size / PB->nbPartition;
          }
        break;
      default:
        /* nothing to do */
        break;
    }

  if (PB->targetOffset == INT32_MAX)
    HandleError (__func__, "device too small", 0, ERR_ABORT);

  if ((PB->order >= 0) && (PB->targetOffset + PB->targetSize > PB->deviceSize))
    {
      char st [MAX_STR];

      sprintf (st, "Adjusting TargetSize (TO = %d, TS = %d DS = %d\n", PB->targetOffset, PB->targetSize, PB->deviceSize);
      PB->targetSize = PB->deviceSize - PB->targetOffset;
      PB->warning = PB->warning | TEST_EXCEED_DEVICE;
      OutputString (OUT_LOG, st);
    }

  if ((PB->order < 0) && (PB->targetOffset - PB->targetSize < 0))
    {
      char st [MAX_STR];

      sprintf (st, "Adjusting TargetSize Reverse Order (TO = %d, TS = %d DS = %d\n", PB->targetOffset, PB->targetSize, PB->deviceSize);
      PB->targetSize = PB->targetOffset;
      PB->warning = PB->warning | TEST_EXCEED_DEVICE;
      OutputString (OUT_LOG, st);
    }
}



/**
 * Parses an experiment plan.
 *
 * @param fp file pointer to an experiment plan
 * @param currExp id of the current experiment
 * @param PB a pointer to a structure containing benchmark parameters for this session
 * @param tabVal an array where parsed values from valueList are stored
 * @return -1 on failure, or the number of parsed values
 */
static int32_t
parseExp (FILE        *fp,
          int32_t      currExp,
          UflipParams *PB,
          int32_t     *tabVal)
{
  char    str [MAX_STR];
  int32_t numExp;
  char    varName [MAX_STR];
  char    varValue [MAX_SIZE_PARAM];
  char    valueList [MAX_STR];
  int32_t nbVal = 0;
  int32_t n;

  rewind (fp);
  numExp = 0; /* Exp 0 does not exists */
  while ((numExp != currExp) && (fgets (str, MAX_STR, fp) != NULL))
    numExp = atoi(str); /* Find the experiment to parse */

  if (numExp != currExp)
    return -1;

  numExp = 0;
  while ((numExp == 0) && (fgets (str, MAX_STR, fp) != NULL))
    {
      /* remove newline */
      if ((str [strlen (str) - 2] == '\x0D') && (str [strlen (str) - 1] == '\x0A'))
        {
          /* windows newline \r\n */
          str[strlen (str) - 2] = '\0';
        }
      else
        {
          /* unix newline \n */
          str[strlen (str) - 1] = '\0';
        }

      numExp = atoi (str);

      /* a line to parse! */
      if (numExp == 0)
        {
          uint32_t pos = 0;
          int32_t  i   = 0;

          /* ignore spaces and tabs */
          while ((pos < strlen (str)) && ((str [pos] == '\x20') || (str [pos] == '\x09')))
            ++pos;

          /* param name */
          while ((pos < strlen (str)) && ((str [pos] != '\x20') && (str [pos] != '\x09')))
            varName [i++] = str [pos++];

          /* check if there is a new parameter */
          if ((i != 0) && (varName [0] != '='))
            {
              varName [i] = '\0';
              i = 0;

              /* ignore spaces and tabs */
              while ((pos < strlen (str)) && ((str [pos] == '\x20') || (str [pos] == '\x09')))
                ++pos;

              /* param value */
              while ((pos < strlen (str)) && ((str [pos] != '\x20') && (str [pos] != '\x09')))
                varValue [i++] = str [pos++];

              varValue [i] = '\0';
              i = 0;

              /* rest of the line */
              while (pos < strlen (str))
                valueList [i++] = str [pos++];

              valueList [i] = '\0';
              n = GetValues (varName, varValue, PB, valueList, tabVal);
              if (n == -1)
                return -1;

              if (n != 0)
                nbVal = n;
            }
        }
    }

  return nbVal;
}



/**
 * Parses an experiment selection.
 *
 * @param PB a pointer to a structure containing benchmark parameters for this session
 * @param tabSel an array where parsed values are stored (true when the
 *               experiment is selected, false otherwise)
 * @return -1 on failure (problem with the experiment selection), 0 on success
 */
static int
parseSel (UflipParams *PB,
          bool        *tabSel)
{
  FILE   *fp = NULL;
  int32_t microB;
  int32_t mode;
  char    str [MAX_STR];

  if ((fp  = fopen (PB->expSelect, "r")) == NULL)
    return -1;

  for (int i = 0; i < MAXBENCH * MAXMODE; ++i)
    tabSel [i]= false;

  while (fgets (str, MAX_STR, fp) != NULL)
    {
      str [1] = '\0';
      microB = atoi (str); /* retrieve a number on 1 digits at the begining */
      str [3] = '\0';
      mode = atoi (&(str [2])); /* retrieve a number on 1 digits at position 2 */
      if ((microB > 0) && (microB <= MAXBENCH) && (mode > 0) && (mode <= MAXMODE))
        tabSel [(microB - 1) * MAXMODE + mode - 1] = true;
    }
  fclose (fp);

  return 0;
}



void
GenPrepare (UflipParams *PB)
{
  FILE   *fp2 = NULL; /* file pointer */
  int32_t size;

  fp2 = fopen (PB->outName, "w");
  if (fp2 == NULL)
    HandleError (__func__, "Could not open output file", errno, ERR_ABORT);

  if (fwrite (SCRIPT_SHEBANGS, sizeof (char), strlen (SCRIPT_SHEBANGS), fp2) < strlen (SCRIPT_SHEBANGS))
    {
      int err = errno;
      fclose (fp2);
      HandleError (__func__, "Could not write to output file", err, ERR_ABORT);
    }
  strcpy (PB->comment, "SPE");
  strcpy (PB->base, "SR");
  PB->expID        = 1;
  PB->microBenchID = 0;
  size = PB->IOSize * PB->IOCount;
  for (PB->runID = 0; PB->runID < PB->nbRun; ++PB->runID)
    {
      PB->targetOffset = uflip_random_get_int (rg, 0, (int32_t) ((PB->deviceSize / 2 - size) / BLOCK)) * BLOCK; /* We choose a random location in the first half of the device */
      PB->targetSize = PB->deviceSize - PB->targetOffset;
      GenExp (fp2, PB);
    }

  PB->IOCount = PB->IOCount2;
  size        = PB->IOSize * PB->IOCount;
  strcpy (PB->comment, "SIO.SR");
  PB->expID   = 2;
  for (PB->runID = 0; PB->runID < PB->nbRun; ++PB->runID)
    {
      PB->targetOffset = uflip_random_get_int (rg, 0, (int32_t) ((PB->deviceSize / 2 - size) / BLOCK)) * BLOCK; /* We choose a random location in the first half of the device */
      PB->targetSize = PB->deviceSize - PB->targetOffset;
      GenExp (fp2, PB);
    }

  strcpy (PB->base, "RR");
  strcpy (PB->comment, "SIO.RR");
  ++PB->expID;
  for (PB->runID = 0; PB->runID < PB->nbRun; ++PB->runID)
    {
      PB->targetOffset = uflip_random_get_int (rg, 0, (int32_t) ((PB->deviceSize / 2 - size) / BLOCK)) * BLOCK; /* We choose a random location in the first half of the device */
      PB->targetSize = PB->deviceSize - PB->targetOffset;
      GenExp (fp2, PB);
    }

  strcpy (PB->base, "SW");
  strcpy (PB->comment, "SIO.SW");
  ++PB->expID;
  for (PB->runID = 0; PB->runID < PB->nbRun; ++PB->runID)
    {
      PB->targetOffset = uflip_random_get_int (rg, 0, (int32_t) ((PB->deviceSize / 2 - size) / BLOCK)) * BLOCK; /* We choose a random location in the first half of the device */
      PB->targetSize = PB->deviceSize - PB->targetOffset;
      GenExp (fp2, PB);
    }

  strcpy (PB->base, "RW");
  strcpy (PB->comment, "SIO.RW");
  ++PB->expID;
  for (PB->runID = 0; PB->runID < PB->nbRun; ++PB->runID)
    {
      PB->targetOffset = uflip_random_get_int (rg, 0, (int32_t) ((PB->deviceSize / 2 - size) / BLOCK)) * BLOCK; /* We choose a random location in the first half of the device */
      PB->targetSize = PB->deviceSize - PB->targetOffset;
      GenExp (fp2, PB);
    }

  ++PB->expID;
  if (PB->fmtType != NONE)
    {
      char *fmt_str;
      if (PB->fmtType == SEQ)
        {
          fmt_str = "SequentialFormat";
        }
      else
        {
          fmt_str = "RandomFormat";
        }
      fprintf (fp2, EXEC_PREFIX""PACKAGE_NAME" %s Dev %s Bench %d Exp %d ", fmt_str, uflip_device_to_string (PB->device), PB->microBenchID, PB->expID);
      if (PB->fake == true)
        fprintf (fp2, "Fake True");
    }
  fprintf (fp2, "\n"SCRIPT_PAUSE"\n");
  fclose (fp2);
}



void
GenBench (UflipParams *PBBench)
{
  bool         finished = false;
  int32_t      prevExp  = 0;
  int32_t      currExp;
  int32_t      numExp;
  FILE        *fp       = NULL;  /* file pointer */
  FILE        *fp2      = NULL;  /* file pointer */
  char         str [MAX_STR];
  int32_t      tabVal [MAX_VARYING_VAL];
  UflipParams *PBExp;
  int32_t      nbVal;
  item        *memList;
  bool         tabSel [MAXBENCH*MAXMODE];
  int32_t      nbExp    = 0;
  int          key      = 0;

  if (parseSel (PBBench, tabSel) == -1)
    HandleError (__func__, "Problem with the experimentation plan!", 0, ERR_ABORT);

  /* Allocate data structure for computing target offset */
  memList = InitMemList ((int32_t) (PBBench->deviceSize));
  if (memList == NULL)
    HandleError (__func__, "Could not allocate memList", errno, ERR_ABORT);

  fp2 = fopen (PBBench->outName, "w");
  if (fp2 == NULL)
    HandleError (__func__, "Could not open output file", errno, ERR_ABORT);

  if (fwrite (SCRIPT_SHEBANGS, sizeof (char), strlen (SCRIPT_SHEBANGS), fp2) < strlen (SCRIPT_SHEBANGS))
    {
      int err = errno;
      fclose (fp2);
      HandleError (__func__, "Could not write to output file", err, ERR_ABORT);
    }

  while (finished == false)
    {
      /* find the next experiment - it is the smallest numExp, greater than prevExp */
      if ((fp = fopen (PBBench->expPlan, "r")) == NULL )
        HandleError (__func__, "Cannot open experimentation plan file", errno, ERR_ABORT);

      currExp = INT32_MAX;
      while (fgets(str, MAX_STR,fp) != NULL)
        {
          numExp = atoi (str);
          if ((numExp > prevExp) && (numExp < currExp))
            currExp = numExp;
        }

      if ((currExp > prevExp) && (currExp != INT32_MAX))
        {
          printf ("===================  Order Number %d\n", currExp);

          /* New param structure */
          PBExp = uflip_params_new ();
          PBExp->device = uflip_device_copy (PBBench->device);
          if (PBExp->device == NULL)
                    HandleError (__func__, "couldn't allocate Param->device subblock", 0, ERR_ABORT);

          InitParams (PBExp);
          PBExp->IOSize       = PBBench->IOSize;
          PBExp->IOCount      = PBBench->IOCount;
          PBExp->IOCountSR    = PBBench->IOCountSR;
          PBExp->IOCountRR    = PBBench->IOCountRR;
          PBExp->IOCountSW    = PBBench->IOCountSW;
          PBExp->IOCountRW    = PBBench->IOCountRW;
          PBExp->ignoreIO     = PBBench->ignoreIO;
          PBExp->ignoreIOSR   = PBBench->ignoreIOSR;
          PBExp->ignoreIORR   = PBBench->ignoreIORR;
          PBExp->ignoreIOSW   = PBBench->ignoreIOSW;
          PBExp->ignoreIORW   = PBBench->ignoreIORW;
          PBExp->collectErase = PBBench->collectErase;
          PBExp->pauseExp     = PBBench->pauseExp;
          PBExp->fake         = PBBench->fake;
          PBExp->bufferType   = PBBench->bufferType;
          PBExp->deviceSize   = PBBench->deviceSize;
          PBExp->burstIO      = PBBench->burstIO;
          PBExp->nbRun        = PBBench->nbRun;

          /* parse the experiment */
          nbVal = parseExp (fp, currExp, PBExp, tabVal);
          if (nbVal == -1)
            HandleError (__func__, "Problem with the experimentation selection!", 0, ERR_ABORT);

          if (tabSel [(PBExp->microBenchID - 1) * MAXMODE + PBExp->expID - 1] == true)
            {
              for (int32_t exp = 0; exp < nbVal; ++exp)
                {
                  /* Compute the different parameters */
                  if (PBExp->microBenchID == PAR)
                    {
                      for (PBExp->runID = 0; PBExp->runID < PBExp->nbRun; ++PBExp->runID)
                        {
                          PBExp->parDeg = tabVal [exp];
                          for (int32_t pID = 0; pID < PBExp->parDeg; ++pID)
                            {
                              PBExp->processID = PBExp->parDeg - pID - 1;
                              PBExp->key       = key++;
                              ComputeParams (PBExp, memList, PBExp->parDeg, nbVal);
                              GenExp (fp2, PBExp);
                              ++nbExp;
                              PBExp->IOSize    = PBBench->IOSize;
                              PBExp->IOCount   = PBBench->IOCount;
                              PBExp->ignoreIO  = PBBench->ignoreIO;
                            }
                        }
                    }
                  else
                    {
                      for (PBExp->runID = 0; PBExp->runID < PBExp->nbRun; ++PBExp->runID)
                        {
                          PBExp->key      = key++;
                          ComputeParams (PBExp, memList, tabVal [exp], nbVal);
                          GenExp (fp2, PBExp);
                          ++nbExp;
                          PBExp->IOSize   = PBBench->IOSize;
                          PBExp->IOCount  = PBBench->IOCount;
                          PBExp->ignoreIO = PBBench->ignoreIO;
                        }
                    }
                }
            }
          else
            {
              printf ("===================  Not Selected\n");
            }
          fprintf (fp2, "\n");
        }
      else
        {
          finished = true;
        }
      prevExp = currExp;
      uflip_params_destroy (PBExp);
      PBExp = NULL;
      if (fp)
        fclose (fp);

    }
  fclose (fp2);

  while (memList)
    {
      item *tmp = memList->next;
      free (memList);
      memList = tmp;
    }

  sprintf (str, "%d Experiments have been generated\n", nbExp);
  OutputString (OUT_LOG, str);
}



static int
max (int a,
     int b)
{
  return a > b ? a : b;
}
