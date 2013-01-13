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
#include <time.h>

#include "genBench.h"
#include "output.h"
#include "uflip_timer.h"

/** array containing all ouput filenames */
char OutputFileName [MAX_OUTPUT_FILES] [MAX_FILENAME];
/** default prefix of the output files */
#define OUTPUTPREFIX ""

static int compare_doubles (const void *a,
                            const void *b);

/* taken from http://www.gnu.org/s/libc/manual/html_node/Comparison-Functions.html#Comparison-Functions */
static int
compare_doubles (const void *a,
                 const void *b)
{
  const double *da = (const double *) a;
  const double *db = (const double *) b;

  return (*da > *db) - (*da < *db);
}



void
InitLogName (const char *str)
{
  strcpy (OutputFileName [OUT_LOG], str);
}



void
InitFileNames (UflipParams *PB)
{
    char base [MAX_STR];

  sprintf (base, "%01d_%01d", PB->microBenchID, PB->expID);
  sprintf (OutputFileName [OUT_RES], "%sRESULTS"DIR_SEPARATOR"RES_%s.csv", OUTPUTPREFIX, base);
  sprintf (base + strlen (base), "_%02d", PB->processID);
  sprintf (OutputFileName [OUT_ALL], "%sTIMINGS"DIR_SEPARATOR"TIM_%s.csv", OUTPUTPREFIX, base);
  sprintf (OutputFileName [OUT_AVG], "%sTIMINGS"DIR_SEPARATOR"AVG_%s.csv", OUTPUTPREFIX, base);
  sprintf (OutputFileName [OUT_SOR], "%sTIMINGS"DIR_SEPARATOR"SOR_%s.csv", OUTPUTPREFIX, base);
  sprintf (base + strlen (base), "_%04d", PB->key);
  switch (PB->microBenchID)
    {
      case 0:
        sprintf (OutputFileName [OUT_COL], "%s", PB->comment);
        break;
      case GRA:
        sprintf (OutputFileName [OUT_COL], "P%.1f", (double) PB->IOSize / (1024.0 / (double) SECTOR));
        break;
      case ALI:
        sprintf (OutputFileName [OUT_COL], "P%.1f", (double) PB->IOShift / (1024.0 / (double) SECTOR));
        break;
      case LOC:
        sprintf (OutputFileName [OUT_COL], "P%.0f", (double) PB->targetSize / (1024.0 / (double) SECTOR));
        break;
      case PAT:
        sprintf (OutputFileName [OUT_COL], "P%03d", PB->nbPartition);
        break;
      case ORD:
        sprintf (OutputFileName [OUT_COL], "P%03d", PB->order);
        break;
      case PAR:
        sprintf (OutputFileName [OUT_COL], "P%02d", PB->parDeg);
        break;
      case MIX:
        sprintf (OutputFileName [OUT_COL], "P%02d", PB->ratio);
        break;
      case PIO:
        sprintf (OutputFileName [OUT_COL], "P%05d", PB->pauseIO);
        break;
      case PBU:
        sprintf (OutputFileName [OUT_COL], "P%06d", PB->burstIO);
        break;
      default:
        break;
    }
  sprintf (base + strlen (base), "_%s_%01d", OutputFileName [OUT_COL], PB->runID);
  sprintf (OutputFileName [OUT_ERR], "%sTRACES"DIR_SEPARATOR"ERR_%s.txt", OUTPUTPREFIX, base);
  sprintf (OutputFileName [OUT_TMP], "%sTRACES"DIR_SEPARATOR"TMP_%s.txt", OUTPUTPREFIX, base);
  sprintf (OutputFileName [OUT_TIM],"%sDETAILS"DIR_SEPARATOR"DET_%s.csv", OUTPUTPREFIX, base);
  sprintf (OutputFileName [OUT_TRA], "%sTRACES"DIR_SEPARATOR"TRA_%s.txt", OUTPUTPREFIX, base);
}



void
OutputString (int         numFile,
              const char *str)
{
  FILE *fp = NULL;

  fp = fopen (OutputFileName [numFile], "a+");
  if (!fp)
    {
      /* try to log the failure */
      if (numFile != OUT_LOG)
        fp = fopen (OutputFileName [OUT_LOG], "a+");

      if ((numFile == OUT_LOG) || (fp == NULL))
        {
          printf ("Cannot output error messages - exiting\n");
          printf ("%s\n", str);
          exit (2);
        }
    }
  if (numFile == OUT_LOG)
    {
      time_t lTime = 0;
      static char cpTime [1024] = "";

      time (&lTime);  /* get the current time */
      strcpy (cpTime, ctime (&lTime));
      cpTime [strlen (cpTime) - 1] = 0; /* ctime adds a trailing newline which we don't want */
      fprintf (fp, "[%s] - %s", cpTime, str);
    }
  else
    {
      fprintf (fp, "%s", str);
    }
  fclose (fp);
}



void
OutputResults (UflipParams  *PB,
               UflipResults *RB)
{
  FILE   *fp    = NULL;  /* file pointer */
  FILE   *fp2   = NULL;
  char    str [MAX_STR];
  bool    Empty = true;
  double  runSum;
  int32_t reinitVal;
  char    res [MAX_STR];

  /* Check if open for read works ==> Not new */
  if ((fp = fopen (OutputFileName [OUT_RES], "r")) != NULL)
    {
      fclose (fp);
      fp = NULL;
      Empty = false;
    }
  else if (errno != ENOENT)
    {
      Empty = false;
    }

  while ((fp = fopen (OutputFileName [OUT_RES], "a+")) == NULL)
    printf ("Could not open result file, %d\n Retrying .... ", errno);

  if (fseek (fp, 0, SEEK_END) == -1)
    {
      int err = errno;
      fclose (fp);
      HandleError (__func__, "Could not open RES file", err, ERR_ABORT);
    }

  if (Empty)
    {
      sprintf (res, "Comment; Key; WG; RunID; PDeg; ProcID; Base; Base1; Ratio; TS2; Order; NbPart; Shift; TSize; PauseIO; Burst; IoSize; "); /* IN (17 cols) */
      strcat (res, "AvgIO; MinIO; MaxIO; stdD; Erase; InitEr; EndEr; Lost; Time; User; Kernel; ");                                            /* OUT (11 cols) */
      strcat (res, "IgnIO; Offset; pauseBurst; CompName; TimeStamp;  PauseExp; Erase; BenchID; ExpID; Buff; Device; TotFile; IOC; TO2 \n");   /* Details (12 cols) */
    }
  else
    {
      strcpy (res, "");
    }

  /* IN (17 col) */
  sprintf (res + strlen (res), "%s; ",   PB->comment);                                          /* identify the experiment */
  sprintf (res + strlen (res), "%4d; ",  PB->key);                                              /* Key */
  sprintf (res + strlen (res), "%1d; ",  PB->warning);                                          /* warning */
  sprintf (res + strlen (res), "%3d; ",  PB->runID);                                            /* RunID between 0 and nbRun */
  sprintf (res + strlen (res), "%d; ",   PB->parDeg);                                           /* varying 6 */
  sprintf (res + strlen (res), "%d; ",   PB->processID);                                        /* reference */
  sprintf (res + strlen (res), "%s; ",   PB->base);
  sprintf (res + strlen (res), "%s; ",   PB->base2);
  sprintf (res + strlen (res), "%d; ",   PB->ratio);                                            /* Varying 7 */
  sprintf (res + strlen (res), "%.1f; ", (double) PB->targetSize2 / (1024. / (double) SECTOR));
  sprintf (res + strlen (res), "%d; ",   PB->order);                                            /* Varying 5 */
  sprintf (res + strlen (res), "%d; ",   PB->nbPartition);                                      /* Varying 4 */
  sprintf (res + strlen (res), "%.1f; ", (double) PB->IOShift / (1024. / (double) SECTOR));     /* Varying 2 */
  sprintf (res + strlen (res), "%.1f; ", (double) PB->targetSize / (1024. / (double) SECTOR));  /* Varying 3 */
  sprintf (res + strlen (res), "%d; ",   PB->pauseIO);                                          /* Varying 8 */
  sprintf (res + strlen (res), "%d; ",   PB->burstIO);                                          /* Varying 9 */
  sprintf (res + strlen (res), "%.1f; ", (double) PB->IOSize / (1024. / (double) SECTOR));      /* Varying 1 */

  /* OUT (11 col) */
  sprintf (res + strlen (res), "%d; ", (uint32_t) (RB->avgIO * 1000000.));     /* Result */
  sprintf (res + strlen (res), "%d; ", (uint32_t) (RB->minIO * 1000000.));
  sprintf (res + strlen (res), "%d; ", (uint32_t) (RB->maxIO * 1000000.));
  sprintf (res + strlen (res), "%d; ", (uint32_t) (RB->stdDevIO* 1000000.));
  sprintf (res + strlen (res), "%d; ", RB->eraseCount);
  sprintf (res + strlen (res), "%d; ", RB->initErase);
  sprintf (res + strlen (res), "%d; ", RB->endErase);
  sprintf (res + strlen (res), "%d; ", (uint32_t) (RB->lostTime  * 1000000.));
  sprintf (res + strlen (res), "%d; ", (uint32_t) (RB->timeClock * 1000000.));
  sprintf (res + strlen (res), "%d; ", (uint32_t) (RB->timeUser  * 1000000.));
  sprintf (res + strlen (res), "%d; ", (uint32_t) (RB->timeKernel* 1000000.));

  /* Details (12 col) */
  sprintf (res + strlen (res), "%d; ",   PB->ignoreIO);
  sprintf (res + strlen (res), "%.1f; ", (double) PB->targetOffset / (1024./ (double) SECTOR));
  sprintf (res + strlen (res), "%d; ",   PB->pauseBurst);
  sprintf (res + strlen (res), "%s; ",   PB->machine);
  sprintf (res + strlen (res), "%s; ",   PB->timeStamp);
  sprintf (res + strlen (res), "%d; ",   PB->pauseExp / 1000);
  if (PB->collectErase == INT32_MAX)
    {
      strcat (res, "NO; ");
    }
  else
    {
      sprintf (res + strlen (res), "%d; ", PB->collectErase);
    }
  sprintf (res + strlen (res), "%d; ", PB->microBenchID);
  sprintf (res + strlen (res), "%d; ", PB->expID);

  switch (PB->bufferType)
    {
      case NO_BUFFERING:
        strcat (res, "NoBuff; ");
        break;
      case FS_BUFFERING:
        strcat (res, "FSBuff; ");
        break;
      case HW_BUFFERING:
        strcat (res, "HWBuff; ");
        break;
      case (FS_BUFFERING | HW_BUFFERING):
        strcat (res, "HW_SW; ");
        break;
      default:
        strcat (res, "????; ");
        break;
    }
  /* DEV */
  sprintf (res + strlen (res), "%s; ",   uflip_device_to_string (PB->device));
  sprintf (res + strlen (res), "%d; ",   PB->deviceSize);
  sprintf (res + strlen (res), "%d; ",   PB->IOCount);
  sprintf (res + strlen (res), "%.1f; ", (double) PB->targetOffset2 / (1024. / (double) SECTOR));

  strcat (res, "\n");
  if (fwrite (res, sizeof (char), strlen (res), fp) < strlen (res))
    {
      int err = errno;
      fclose (fp);
      HandleError (__func__, "Could not write to result file.", err, ERR_ABORT);
    }
  OutputString (OUT_TRA, res);

  fclose (fp);

  /* OUTPUT GLOBAL TIMINGS */
  if ((fp = fopen (OutputFileName [OUT_ALL], "r")) != NULL)
    {
      fp2 = fopen (OutputFileName [OUT_TMP], "a+");
      if (fp2 == NULL)
        HandleError (__func__, "Could not open TMP file", errno, ERR_ABORT);

      if (fgets (str, MAX_STR, fp) == NULL)
        {
          int err = errno;
          fclose (fp2);
          fclose (fp);
          HandleError (__func__, "Could not read global timings", err, ERR_ABORT);
        }
      str [strlen (str) - 1] = '\0';
      fprintf (fp2, "%s; %s\n", str, OutputFileName [OUT_COL]);
      for (int32_t it = 0; it < PB->IOCount; ++it)
        {
          if (fgets (str, MAX_STR, fp) == NULL)
            sprintf (str, " \n");

          str [strlen (str) - 1] = '\0';
          fprintf (fp2, "%s; %d\n", str, (uint32_t) (RB->timing [it] * 1000000.));
        }
      fclose (fp2);
      fclose (fp);
      remove (OutputFileName [OUT_ALL]);
      rename (OutputFileName [OUT_TMP], OutputFileName [OUT_ALL]);
      remove (OutputFileName [OUT_TMP]);
    }
  else
    {
      fp = fopen (OutputFileName [OUT_ALL], "a+");
      fprintf (fp, "NUMIO; %s\n", OutputFileName [OUT_COL]);
      for (int32_t it = 0; it < PB->IOCount; ++it)
        fprintf (fp, "%d; %d\n", it, (uint32_t) (RB->timing [it] * 1000000.));

      fclose (fp);
    }

  /* OUTPUT RUNNING AVG */
  if ((fp = fopen (OutputFileName [OUT_AVG], "r")) != NULL)
    {
      fp2 = fopen (OutputFileName [OUT_TMP], "a+");
      if (fp2 == NULL)
        HandleError (__func__, "Could not open TMP file", errno, ERR_ABORT);

      if (fgets (str, MAX_STR, fp) == NULL)
        {
          int err = errno;
          fclose (fp2);
          fclose (fp);
          HandleError (__func__, "Could not read running avg", err, ERR_ABORT);
        }
      str [strlen (str) - 1] = '\0';
      fprintf (fp2, "%s; %s\n", str, OutputFileName [OUT_COL]);
      runSum = 0;
      reinitVal = 0;
      for (int32_t it = PB->ignoreIO; it < PB->IOCount; ++it)
        {
          if ((strcasecmp (PB->comment, "SPE") == 0)  && ((it == PB->IOCount / 3) || (it == 2 * PB->IOCount / 3)))
            {
              runSum = 0;
              reinitVal += PB->IOCount / 3;
            }
          runSum += RB->timing [it];
          if (fgets (str, MAX_STR, fp) == NULL)
            sprintf (str, " \n");

          str [strlen (str) - 1] = '\0';
          fprintf (fp2, "%s; %d\n", str, (uint32_t) (runSum / ((double) (it - PB->ignoreIO - reinitVal + 1)) * 1000000.));
        }
      fclose (fp2);
      fclose (fp);
      remove (OutputFileName [OUT_AVG]);
      rename (OutputFileName [OUT_TMP], OutputFileName [OUT_AVG]);
      remove (OutputFileName [OUT_TMP]);
    }
  else
    {
      fp = fopen (OutputFileName [OUT_AVG], "a+");
      fprintf (fp, "NUMIO; %s\n", OutputFileName [OUT_COL]);
      runSum = 0;
      reinitVal = 0;
      for (int32_t it = PB->ignoreIO; it < PB->IOCount; ++it)
        {
          if ((strcasecmp (PB->comment, "SPE") == 0)  && ((it == PB->IOCount / 3) || (it == 2 * PB->IOCount / 3)))
            {
              runSum = 0;
              reinitVal += PB->IOCount / 3;
            }
          runSum += RB->timing [it];
          fprintf (fp, "%d; %d\n", it, (uint32_t) (runSum / ((double) (it - PB->ignoreIO - reinitVal + 1))  * 1000000.));
        }
      fclose (fp);
    }

  /* output detailed timing with erase and type */
  fp = fopen (OutputFileName [OUT_TIM], "a+");
  if (!fp)
    HandleError (__func__, "Could not open TIMING file", errno, ERR_ABORT);

  for (int32_t it = 0; it < PB->IOCount; ++it)
    fprintf (fp, "%c; %c; %d; %ld\n", RB->isRND [it] ? 'R' : 'S', RB->isWrite [it] ? 'W': 'R', RB->dPos [it], (long) (RB->timing [it] * 1000000.));

  fclose (fp);

  /* SORT TIMINGS */
  qsort (RB->timing, PB->IOCount, sizeof (double), compare_doubles);

  /* OUTPUT SORTED GLOBAL TIMINGS */
  if ((fp = fopen (OutputFileName [OUT_SOR], "r")) != NULL )
    {
      fp2 = fopen (OutputFileName [OUT_TMP], "a+");
      if (fp2 == NULL)
        HandleError (__func__, "Could not open TMP file", errno, ERR_ABORT);

      if (fgets (str, MAX_STR, fp) == NULL)
        {
          int err = errno;
          fclose (fp2);
          fclose (fp);
          HandleError (__func__, "Could not read sorted global timings", err, ERR_ABORT);
        }
      str [strlen (str) - 1] = '\0';
      fprintf (fp2, "%s; %s\n", str, OutputFileName [OUT_COL]);
      for (int32_t it = 0; it < PB->IOCount; ++it)
        {
          if (fgets (str, MAX_STR,fp) == NULL)
            sprintf (str, " \n");

          str [strlen (str) - 1] = '\0';
          fprintf (fp2, "%s; %d\n", str, (uint32_t) (RB->timing [it] * 1000000.));
        }
      fclose (fp2);
      fclose (fp);
      remove (OutputFileName [OUT_SOR]);
      rename (OutputFileName [OUT_TMP], OutputFileName [OUT_SOR]);
      remove (OutputFileName [OUT_TMP]);
    }
  else
    {
      fp = fopen (OutputFileName [OUT_SOR], "a+");
      fprintf (fp, "NUMIO; %s\n", OutputFileName [OUT_COL]);
      for (int32_t it = 0; it < PB->IOCount; ++it)
        fprintf (fp, "%d;%d\n", it, (uint32_t) (RB->timing [it] * 1000000.));

      fclose (fp);
    }
}



void
OutputLine (const char *buffer)
{
  OutputString (OUT_ERR, buffer);
  OutputString (OUT_LOG, buffer);
  OutputString (OUT_TRA, buffer);
  fprintf (stderr, "%s\n", buffer); /* outputs to console */
}



void
HandleError (const char  *caller,
             const char  *error,
             int          errNum,
             enum err_lvl errLevel)
{
  char buffer [MAX_STR] = "";

  sprintf (buffer, "%s: %s (%d)\n", caller,  error,  errNum);
  OutputLine (buffer);
  if (errLevel == ERR_ABORT)
    {
      OutputLine ("Exiting...\n");
      uflip_wait_microsec (2000);
      exit (1);  /* kills the program */
    }
}
