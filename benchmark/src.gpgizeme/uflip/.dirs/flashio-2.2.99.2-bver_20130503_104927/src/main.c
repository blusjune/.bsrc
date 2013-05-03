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
#include <string.h>

#include "genBench.h"
#include "input.h"
#include "microbench.h"
#include "output.h"
#include "rndFormat.h"
#include "seqFormat.h"
#include "uflip_random.h"

uint32_t     seed = 200; /**< random seed */
UflipRandom *rg;         /**< common random number generator */

static void RunParBench (UflipParams *PB);

/**
 * Provides special synchronization when parallel instances are running.
 *
 * @param PB pointer to a structure containing parameters used for the current benchmark session
 */
static void
RunParBench (UflipParams *PB)
{
  UflipResults *RB;            /* output results */
  char          str [MAX_STR]; /* Temporary string */
  int           pi;
  FILE         *fp;
  bool          aSem;

  if(PB->processID != 0)
    {
      /* This is a Slave process */
      printf("WAITING MAINProc TO START .... ");  /* Should wait the master process */
      pi = 0;

      while ((fp = fopen ("_SEMAPHORE", "r")) == NULL)
        { /* Synchronization is done using files */
          ++pi;
          if (pi == 1000)
            {
              printf (".");
              pi = 0;
            }
        }
      printf ("STARTING \n");
      fclose (fp);
      fp = NULL;
      sprintf (str, "_SEM_%d", PB->processID); /* Create a "file" semaphore */
      if ((fp = fopen (str, "w")) == NULL)
        {
          HandleError (__func__, "Cannot create SEMi file", errno, ERR_ABORT);
        }
      fclose (fp);
    }
  else
    {
      /* MASTER */
      /* Before starting, Master checks that no slave semaphore is still present (ie. previous execution ended correctly) */
      printf ("CHECKING THAT ALL INSTANCES OF PREVIOUS EXP HAVE ENDED \n");
      aSem = true;
      while (aSem == true)
        {
          aSem = false;
          for (int32_t nf = 0; nf < MAX_PARALLEL; ++nf)
            {
              sprintf (str, "_SEM_%d", nf); /* Master checks that no slave semaphore is still present */
              if ((fp  = fopen (str, "r")) != NULL )
                {
                  aSem = true;
                  fclose (fp);
                  fp = NULL;
                }
            }
        }
      printf ("CHECKED\n");
      pi = 0;
      /* Now it can start! put his semaphore */
      printf ("STARTING (CREATING _SEMAPHORE FILE)\n");
      while ((fp = fopen ("_SEMAPHORE", "w")) == NULL)
        {
          ++pi;
          if (pi == 100)
            {
              printf (".");
              pi = 0;
            }
        }
      printf ("STARTING !\n");
      fclose (fp);
      fp = NULL;
    }

  /* Finally run the bench */
  RB = runBench (PB);
  OutputResults (PB, RB);
  if (PB->processID == 0)
    {
      /* MASTER */
      printf ("WAITING FOR THE END OF ALL INSTANCES \n");
      aSem = true;
      while (aSem == true)
        {
          aSem = false;
          for (int32_t nf = 0; nf < MAX_PARALLEL; ++nf)
            {
              sprintf (str, "_SEM_%d", PB->processID);
              if ((fp  = fopen (str, "r")) != NULL)
                {
                  aSem = true;
                  fclose (fp);
                  fp = NULL;
                }
            }
        }
      printf ("ENDED\n");
      printf ("REMOVING _SEMAPHORE .... ");
      pi = 0;
      while (remove ("_SEMAPHORE") != 0)
        {
          pi++;
          if (pi == 100)
            {
              printf (".");
              pi = 0;
            }
        }
      printf ("REMOVED");
    }
  else
    {
      /* SLAVE */
      sprintf (str, "_SEM_%d", PB->processID);
      printf ("REMOVING %s .... ", str);
      pi = 0;
      while (remove (str) != 0)
        {
          ++pi;
          if (pi == 100)
            {
              printf (".");
              pi = 0;
            }
        }
      printf ("REMOVED");
    }

  uflip_results_destroy (RB);
  RB = NULL;
}



/**
 * Main of FlashIO - read parameters and trigger appropriate actions.
 *
 * @param argc number of arguments
 * @param argv list of arguments
 * @return status code
 */
int
main (int    argc,
      char **argv)
{
  rg = uflip_random_new (seed); /* make instance of random number generator */
  UflipParams  *PB = NULL;      /* Input parameters */
  UflipResults *RB = NULL;      /* Output results */
  char          str [MAX_STR];  /* Temporary string */

  printf (PACKAGE_STRING" ©Luc Bouganim - 2008-2009  See www.uflip.org\n");
  InitLogName ("TRACES"DIR_SEPARATOR"LOG.txt");
  sprintf (str, "==>  ");
  for (int yy = 0; yy < argc; ++yy)
    {
      sprintf (str + strlen (str), " %s", argv[yy]);
    }
  strcat (str, "\n");
  OutputString (OUT_LOG, str);
  PB = GetTestParameters (argc, argv);
  //PB->fake = true;
  InitFileNames (PB);
  OutputString (OUT_TRA, str);
  if (PB->deviceSize == 0)
    {
      uint64_t *dg = NULL;  /* disk drive geometry structure */
      dg = uflip_device_get_geometry (PB->device);
      if (dg)
        {
          sprintf (str, "Cylinders = %"PRId64"\n", dg [0]);
          OutputString (OUT_TRA, str);
          sprintf (str, "Tracks/cylinder = %"PRId64"\n", dg [1]);
          OutputString (OUT_TRA, str);
          sprintf (str, "Sectors/track = %"PRId64"\n", dg [2]);
          OutputString (OUT_TRA, str);
          sprintf (str, "Bytes/sector = %"PRId64"\n", dg [3]);
          OutputString (OUT_TRA, str);
          PB->deviceSize = dg [0] * dg [1] * dg [2] * dg [3] / SECTOR;
#if 1 /* blusjune */
	  printf(">>> (1) PB->deviceSize: %d\n", PB->deviceSize);
#endif
          sprintf (str, "Disk size = %20.0f (Sectors) = %d (MB)\n", (float) PB->deviceSize, PB->deviceSize / (2 * 1024));
          OutputString (OUT_TRA, str);
          PB->deviceSize = ((uint32_t) (PB->deviceSize / BLOCK)) * BLOCK; /* Use only a size aligned on block */
#if 1 /* blusjune */
	  printf(">>> (2) PB->deviceSize: %d\n", PB->deviceSize);
#endif
          free (dg);
          dg = NULL;
        }
      else
        {
          HandleError (__func__, "GetDriveGeometry failed", 0, ERR_ABORT);
        }
    }
  switch (PB->mainFunction)
    {
      case SEQ_FORMAT:
        seqFormat (PB);
        break;
      case RND_FORMAT:
        rndFormat (PB);
        break;
      case GEN_PREPARE:
        GenPrepare (PB);
        break;
      case GEN_BENCH:
        GenBench (PB);
        break;
      case RUN_BENCH:
        if (PB->parDeg > 1)
          {
            RunParBench (PB);
          }
        else
          {
            RB = runBench (PB);
            OutputResults (PB, RB);
          }
        break;
      default:
        PrintHelp ("");
        break;
    }
  uflip_results_destroy (RB);
  RB = NULL;
  uflip_params_destroy (PB);
  PB = NULL;

  return 0;
}
