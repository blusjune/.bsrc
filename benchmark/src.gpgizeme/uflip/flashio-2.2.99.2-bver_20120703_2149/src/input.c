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

#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include "input.h"
#include "output.h"
#include "os/functions.h"

static const char *functions [6] [2] =
{
  {"RandomFormat",     "Writes entirely the device (random IOs of random sizes)"},
  {"SequentialFormat", "Writes entirely the device (sequential IOs of 128K)"},
  {"GenPrepare",       "Generates a batch to prepare the bench"},
  {"GenBench",         "Generates a batch running all benchmark experiments"},
  {"Run",              "Runs a single experiment"},
  {NULL,               NULL}
};

static const char *common_options [4] [3] =
{
  {"Dev",  "<device number>", "Device number where tests are performed"},
  {"Size", "<Number Blocks>", "Number of Block of the device (detected if not specified)"},
  {"Fake", "<True/False>",    "Run fake tests (no real IOs) - To check"},
  {NULL,   NULL,              NULL}
};

static const char *rnd_options [3] [3] =
{
  {"Bench", "<id>", "Bench ID (should be 0 = default)"},
  {"Exp",   "<id>", "Experiment ID (should be 6 = default)"},
  {NULL,    NULL,   NULL}
};

static const char *seq_options [3] [3] =
{
  {"Bench", "<id>", "Bench ID (should be 0 = default)"},
  {"Exp",   "<id>", "Experiment ID (should be 7 = default)"},
  {NULL,    NULL,   NULL}
};

static const char *genprep_options [9] [3] =
{
  {"OutName",    "<filename>",       "Filename for generating the bench"},
  {"NbRun",      "<n>",              "Number of runs for each experiment"},
  {"IOS",        "<IO size>",        "IO request size in sectors (512 Bytes)"},
  {"IOC",        "<IO Count>",       "Number of IOs, in blocks, for determining IgnoreIO"},
  {"IOC2",       "<IO Count>",       "Number of IOs, in blocks, for det. Startup and Period"},
  {"CErase",     "<address>",        "If Erase count should be collected, where"},
  {"Pause",      "<nb of ms>",       "Pause between exps. (a priori)"},
  {"FormatType", "<[None|Seq|Rnd]>", "Enumerated type of format (Rnd if not specified)"},
  {NULL,         NULL,               NULL}
};

static const char *genbench_options [17] [3] =
{
  {"ExpPlan",    "<filename>",  "File containing the experimentation plan"},
  {"ExpSel",     "<filename>",  "File containing the selection of bench to generate"},
  {"OutName",    "<filename>",  "Filename for generating the bench"},
  {"NbRun",      "<n>",         "Number of runs for each experiment"},
  {"IOS",        "<IO size>",   "IO request size in sectors (512 Bytes)"},
  {"IOCSR",      "<IO Count>",  "Number of IOs, in blocks, for SR"},
  {"IOCRR",      "<IO Count>",  "Number of IOs, in blocks, for RR"},
  {"IOCSW",      "<IO Count>",  "Number of IOs, in blocks, for SW"},
  {"IOCRW",      "<IO Count>",  "Number of IOs, in blocks, for RW"},
  {"IgnIOSR",    "<Nb IO>",     "Number of IOs to ignore for SR"},
  {"IgnIORR",    "<Nb IO>",     "Number of IOs to ignore for RR"},
  {"IgnIOSW",    "<Nb IO>",     "Number of IOs to ignore for SW"},
  {"IgnIORW",    "<Nb IO>",     "Number of IOs to ignore for RW"},
  {"CErase",     "<address>",   "If Erase count should be collected, where"},
  {"Pause",      "<nb of ms>",  "Pause between exps. (a priori)"},
  {"BufferType", "<[N|H|S|A]>", "Enumerated buffering option"},
  {NULL,         NULL,          NULL}
};

static const char *run_options [28] [3] =
{
  {"IOS",        "<IO size>",       "IO request size in sectors (512 Bytes)"},
  {"IOC",        "<IO Count>",      "Number of IOs, in blocks, for one test"},
  {"TSize",      "<targetsize>",    "In sectors for Locality(3) and Circularity(4)"},
  {"TOffs",      "<offset>",        "In sectors offset at which tests will begun"},
  {"TSize2",     "<targetsize>",    "In sectors for Locality(3) and Circularity(4) (MIX pattern)"},
  {"TOffs2",     "<offset>",        "In sectors offset at which tests will begun (MIX pattern)"},
  {"Part",       "<Nb partitions>", "Number of partitions for Partitioning(5)"},
  {"Shift",      "<shift (B)>",     "Shift in sectors before IO"},
  {"IgnIO",      "<Nb IO>",         "Number of IOs to ignore before computing stats"},
  {"Base",       "<SR|RR|SW|RW>",   "Baseline pattern (or first one when mix)"},
  {"Base2",      "<SR|RR|SW|RW>",   "Second baseline pattern when mix"},
  {"Ratio",      "<ratio>",         "Nb of pattern1 for 1 pattern 2 (if pos) and conversely if neg)"},
  {"Order",      "<k>",             "Linear increase or decrease (in blocks)"},
  {"PIO",        "<nb of us>",      "Pause between IOs (in microsec)"},
  {"PBurst",     "<nb of us>",      "Pause between burst of burstIO IOs"},
  {"BurstIO",    "<B>",             "Number of IOs in a burst (B)"},
  {"ParDeg",     "<D>",             "Parallel degree"},
  {"PID",        "<n>",             "Process ID between 1 and 16 when parallel IOs"},
  {"CErase",     "<address>",       "If Erase count should be collected, where"},
  {"Pause",      "<nb of ms>",      "Pause between exps. (a priori)"},
  {"BufferType", "<[N|H|S|A]>",     "Enumerated buffering option"},
  {"Bench",      "<id>",            "Bench ID between 0 and 9"},
  {"Exp",        "<id>",            "Experiment ID between 1 and 6"},
  {"Run",        "<id>",            "Run ID between 1 and n"},
  {"Com",        "<comments>",      "Experiment name (user defined string)"},
  {"Warn",       "<ID>",            "Warning ID in case of problem when generating Bench"},
  {"Key",        "<KeyID>",         "Unique key to identify sequentially the experiment"},
  {NULL,         NULL,              NULL}
};

static bool  parse_number          (const char  *ref,
                                    const char  *opt,
                                    const char  *val,
                                    const char  *format,
                                    void        *ptr);
static bool  parse_enum            (const char  *ref,
                                    const char  *opt,
                                    const char  *val,
                                    const char **enum_str,
                                    const void  *enum_val,
                                    size_t       array_size,
                                    void        *ptr,
                                    size_t       val_size);
static bool  get_common_values     (const char  *varName,
                                    const char  *varValue,
                                    UflipParams *PB);
static bool  get_common_id_values  (const char  *varName,
                                    const char  *varValue,
                                    UflipParams *PB);
static bool  get_common_gen_values (const char  *varName,
                                    const char  *varValue,
                                    UflipParams *PB);
static void  GetValuesRF           (const char  *varName,
                                    const char  *varValue,
                                    UflipParams *PB);
static void  GetValuesSF           (const char  *varName,
                                    const char  *varValue,
                                    UflipParams *PB);
static void  GetValuesGP           (const char  *varName,
                                    const char  *varValue,
                                    UflipParams *PB);
static void  GetValuesGB           (const char  *varName,
                                    const char  *varValue,
                                    UflipParams *PB);
static void  InitParamsRF          (UflipParams *PB);
static void  InitParamsSF          (UflipParams *PB);
static void  InitParamsGP          (UflipParams *PB);
static void  InitParamsGB          (UflipParams *PB);
static void  InitParamsR           (UflipParams *PB);
static char *CheckPB               (UflipParams *PB);

/**
 * Parses a number from a string.
 *
 * @param ref expected option name
 * @param opt option name
 * @param val option value
 * @param format format string, passed as is to sscanf
 * @param ptr pointer receiving the parsed value
 * @return false if the option name doesn't match the expected one, true if the parsing succeeded.
 */
static bool
parse_number (const char *ref,
              const char *opt,
              const char *val,
              const char *format,
              void       *ptr)
{
  if (strcasecmp (ref, opt) != 0)
    return false;

  if (sscanf (val, format, ptr) != 1)
    PrintHelp (opt);

  return true;
}



/**
 * Parses an enum from a string.
 *
 * @param ref expected option name
 * @param opt option name
 * @param val option value
 * @param enum_str pointer to an array of possible option values
 * @param enum_val pointer to an array of the corresponding enum values, in the same order
 * @param array_size size of the two previous arrays
 * @param ptr pointer receiving the parsed value
 * @param val_size size of the value pointed by ptr
 * @return false if the option name doesn't match the expected one, true if the parsing succeeded.
 */
static bool
parse_enum (const char  *ref,
            const char  *opt,
            const char  *val,
            const char **enum_str,
            const void  *enum_val,
            size_t       array_size,
            void        *ptr,
            size_t       val_size)
{
  size_t i;
  int    res = -2;

  if (strcasecmp (ref, opt) != 0)
    return false;

  for (i = 0; i < array_size; ++i)
    {
      if ((res = strcasecmp (enum_str [i], val)) == 0)
        break;
    }

  if (res != 0)
    PrintHelp (opt);

  memcpy (ptr, enum_val + val_size * i, val_size);
  return true;
}



/**
 * Parses the following options/values: Dev, Size, Fake. They are common to all functions.
 *
 * @param varName option name
 * @param varValue option value
 * @param PB pointer to a struct where to store the parsed options
 * @return false if given @a varName isn't a known option or if given @a varValue
 * isn't a valid value for the option, otherwise true.
 */
static bool
get_common_values (const char  *varName,
                   const char  *varValue,
                   UflipParams *PB)
{
  const char *fake_str  [2] = { "True", "False" };
  const bool  fake_enum [2] = { true,   false   };

  if (strcasecmp (varName, "Dev") == 0)
    {
      PB->device = uflip_device_new (varValue);
    }
  else if (parse_number ("Size", varName, varValue, "%d", &PB->deviceSize))
    {
      PB->deviceSize /= BLOCK;
      PB->deviceSize *= BLOCK;
    }
  else if (parse_enum ("Fake", varName, varValue, fake_str, fake_enum, 2,
                       &PB->fake, sizeof (PB->fake)));
  else
    {
      return false; /* warning, unknown option! */
    }
  return true; /* option found and parsed. */
}



/**
 * Parses the following options/values: Bench, Exp. They are common to
 * RandomFormat, SequentialFormat and Run.
 *
 * @param varName option name
 * @param varValue option value
 * @param PB pointer to a struct where to store the parsed options
 * @return false if @a varName isn't Bench nor Exp, true otherwise.
 */
static bool
get_common_id_values (const char  *varName,
                      const char  *varValue,
                      UflipParams *PB)
{
  return (parse_number ("Bench", varName, varValue, "%d", &PB->microBenchID) ||
          parse_number ("Exp", varName, varValue, "%d", &PB->expID));
}



/**
 * Parses the following options/values: NbRun, IOS, CErase. They are common to
 * GenPrepare and GenBench.
 *
 * @param varName option name
 * @param varValue option value
 * @param PB pointer to a struct where to store the parsed options
 * @return false if @a varName isn't NbRun nor IOS nor CErase, true otherwise.
 */
static bool
get_common_gen_values (const char  *varName,
                       const char  *varValue,
                       UflipParams *PB)
{
  return (parse_number ("NbRun", varName, varValue, "%d", &PB->nbRun) ||
          parse_number ("IOS", varName, varValue, "%d", &PB->IOSize)  ||
          parse_number ("CErase", varName, varValue, "%d", &PB->collectErase));
}



/**
 * Gets values associated to a given option for the RndFormat function.
 *
 * @param varName NULL-terminated string containing the option name
 * @param varValue NULL-terminated string containing the option value
 * @param PB a pointer to a structure containing benchmark parameters for this session
 */
static void
GetValuesRF (const char  *varName,
             const char  *varValue,
             UflipParams *PB)
{
  if (get_common_values (varName, varValue, PB))
    return;

  /* still some specific options to parse */
  if (get_common_id_values (varName, varValue, PB))
    return;

  /* ok, now this is fatal */
  PrintHelp (varName);
}



/**
 * Gets values associated to a given option for the SeqFormat function.
 *
 * @param varName NULL-terminated string containing the option name
 * @param varValue NULL-terminated string containing the option value
 * @param PB a pointer to a structure containing benchmark parameters for this session
 */
static void
GetValuesSF (const char  *varName,
             const char  *varValue,
             UflipParams *PB)
{
  /* same options as RandomFormat at the moment */
  GetValuesRF (varName, varValue, PB);
}



/**
 * Gets values associated to a given option for the GenPrepare function.
 *
 * @param varName NULL-terminated string containing the option name
 * @param varValue NULL-terminated string containing the option value
 * @param PB a pointer to a structure containing benchmark parameters for this session
 */
static void
GetValuesGP (const char  *varName,
             const char  *varValue,
             UflipParams *PB)
{
  const char *fmt_str  [3] = { "None", "Seq", "Rnd" };
  const int   fmt_enum [3] = { NONE,   SEQ,   RND   };

  if (get_common_values (varName, varValue, PB))
    return;

  /* still some specific options to parse */
  if (get_common_gen_values (varName, varValue, PB))
    return;

  /* still not fatal */
  if (strcasecmp (varName, "OutName") == 0)
    {
      strcpy (PB->outName, varValue);
    }
  else if (parse_number ("IOC", varName, varValue, "%d", &PB->IOCount));
  else if (parse_number ("IOC2", varName, varValue, "%d", &PB->IOCount2));
  else if (parse_number ("Pause", varName, varValue, "%d", &PB->pauseExp))
    {
      PB->pauseExp *= 1000;
    }
  else if (parse_enum ("FormatType", varName, varValue, fmt_str, fmt_enum, 3,
                       &PB->fmtType, sizeof (PB->fmtType)));
  else
    {
      /* ok, now this is fatal */
      PrintHelp (varName);
    }
}



/**
 * Gets values associated to a given option for the GenBench function.
 *
 * @param varName NULL-terminated string containing the option name
 * @param varValue NULL-terminated string containing the option value
 * @param PB a pointer to a structure containing benchmark parameters for this session
 */
static void
GetValuesGB (const char  *varName,
             const char  *varValue,
             UflipParams *PB)
{
  const char *buf_str  [4] = { "N",          "H",          "S",          "A"                         };
  const int   buf_enum [4] = { NO_BUFFERING, HW_BUFFERING, FS_BUFFERING, HW_BUFFERING | FS_BUFFERING };

  if (get_common_values (varName, varValue, PB))
    return;

  /* still some specific options to parse */
  if (get_common_gen_values (varName, varValue, PB))
    return;

  /* still not fatal */
  if (strcasecmp (varName, "ExpPlan") == 0)
    {
      strcpy (PB->expPlan, varValue);
    }
  else if (strcasecmp (varName, "ExpSel") == 0)
    {
      strcpy (PB->expSelect, varValue);
    }
  else if (strcasecmp (varName, "OutName") == 0)
    {
      strcpy (PB->outName, varValue);
    }
  else if (parse_number ("IOCSR", varName, varValue, "%d", &PB->IOCountSR));
  else if (parse_number ("IOCRR", varName, varValue, "%d", &PB->IOCountRR));
  else if (parse_number ("IOCSW", varName, varValue, "%d", &PB->IOCountSW));
  else if (parse_number ("IOCRW", varName, varValue, "%d", &PB->IOCountRW));
  else if (parse_number ("IgnIOSR", varName, varValue, "%d", &PB->ignoreIOSR));
  else if (parse_number ("IgnIORR", varName, varValue, "%d", &PB->ignoreIORR));
  else if (parse_number ("IgnIOSW", varName, varValue, "%d", &PB->ignoreIOSW));
  else if (parse_number ("IgnIORW", varName, varValue, "%d", &PB->ignoreIORW));
  else if (parse_number ("Pause", varName, varValue, "%d", &PB->pauseExp));
  else if (parse_enum ("BufferType", varName, varValue, buf_str, buf_enum, 4,
                       &PB->bufferType, sizeof (PB->bufferType)));
  else
    {
      /* ok, now this is fatal */
      PrintHelp (varName);
    }
}



int32_t
GetValues (const char  *varName,
           const char  *varValue,
           UflipParams *PB,
           char        *valueList,
           int32_t     *tabVal)
{
  const char *buf_str  [4] = { "N",          "H",          "S",          "A"                         };
  const int   buf_enum [4] = { NO_BUFFERING, HW_BUFFERING, FS_BUFFERING, HW_BUFFERING | FS_BUFFERING };
  uint32_t    pos          = 0;
  int32_t     i            = 0;
  char        str      [100];
  int32_t     j            = 0;

  if (!get_common_values (varName, varValue, PB))
    {
      /* not fatal error, still some specific options to parse */
      if (!get_common_id_values (varName, varValue, PB))
        {
          /* still not fatal */
          if (parse_number ("IOS", varName, varValue, "%d", &PB->IOSize));
          else if (parse_number ("Ratio", varName, varValue, "%d", &PB->ratio));
          else if (parse_number ("IOC", varName, varValue, "%d", &PB->IOCount));
          else if (parse_number ("TSize", varName, varValue, "%d", &PB->targetSize));
          else if (parse_number ("TOffs", varName, varValue, "%d", &PB->targetOffset));
          else if (parse_number ("TSize2", varName, varValue, "%d", &PB->targetSize2));
          else if (parse_number ("TOffs2", varName, varValue, "%d", &PB->targetOffset2));
          else if (parse_number ("Part", varName, varValue, "%d", &PB->nbPartition));
          else if (parse_number ("Shift", varName, varValue, "%d", &PB->IOShift));
          else if (parse_number ("IgnIO", varName, varValue, "%d", &PB->ignoreIO));
          else if (strcasecmp (varName, "Base") == 0)
            {
              strcpy (PB->base, varValue);
            }
          else if (strcasecmp (varName, "Base2") == 0)
            {
              strcpy (PB->base2, varValue);
            }
          else if (parse_number ("Order", varName, varValue, "%d", &PB->order));
          else if (parse_number ("PIO", varName, varValue, "%d", &PB->pauseIO));
          else if (parse_number ("PBurst", varName, varValue, "%d", &PB->pauseBurst));
          else if (parse_number ("BurstIO", varName, varValue, "%d", &PB->burstIO));
          else if (parse_number ("ParDeg", varName, varValue, "%d", &PB->parDeg));
          else if (parse_number ("PID", varName, varValue, "%d", &PB->processID));
          else if (parse_number ("CErase", varName, varValue, "%d", &PB->collectErase));
          else if (parse_number ("Pause", varName, varValue, "%d", &PB->pauseExp))
            {
              PB->pauseExp *= 1000;
            }
          else if (parse_enum ("BufferType", varName, varValue, buf_str, buf_enum, 4,
                               &PB->bufferType, sizeof (PB->bufferType)));
          else if (parse_number ("Bench", varName, varValue, "%d", &PB->microBenchID));
          else if (parse_number ("Warn", varName, varValue, "%d", &PB->warning));
          else if (parse_number ("Run", varName, varValue, "%d", &PB->runID));
          else if (parse_number ("Key", varName, varValue, "%d", &PB->key));
          else if (strcasecmp (varName, "Com") == 0)
            {
              strcpy (PB->comment, varValue);
            }
          else
            {
              /* ok, now this is fatal */
              PrintHelp (varName);
            }
        }
    }

  if (valueList == NULL)
    return 0;

  while (pos < strlen (valueList))
    {
      while ((pos < strlen (valueList)) && ((valueList [pos] != '-') && (!isdigit (valueList [pos]))))
        ++pos; /* ignore any non numeric value */

      while ((pos < strlen (valueList)) && ((valueList [pos] == '-') || isdigit (valueList[pos])))
        str [i++] = valueList [pos++]; /* copy numbers into str */

      str [i] = '\0';
      if (strlen (str) > 0)
        {
          if (j + 1 < MAX_VARYING_VAL)
            {
              tabVal [j++] = atol (str);
            }
          else
            {
              return -1;
            }
          /*printf("[%"PRId32"] ", tabVal[j-1]);*/
        }
      i = 0;
    }
  /*printf("\n");*/
  return j;
}



/**
 * Proceeds sanity checks on the given structure.
 *
 * @param PB a pointer to a structure containing benchmark parameters for this session
 * @return NULL if all sanity checks passed, the error string otherwise (to be
 * free(3) by the caller).
 */
static char
*CheckPB (UflipParams *PB)
{
  char err [MAX_STR] = "";

  if (PB->IOSize <= 0)
    {
      sprintf (err, "Err. param. IOSize value: %d", PB->IOSize);
    }
  else if (PB->IOCount <= 0)
    {
      sprintf (err, "Err. param. IOCount value: %d", PB->IOCount);
    }
  else if (PB->IOCount <= PB->ignoreIO)
    {
      sprintf (err, "%s", "IoCount is less than ignoreIO!");
    }
  else if (PB->IOCount2 < 0)
    {
      sprintf (err, "Err. param. IOCount2 value: %d", PB->IOCount2);
    }
  else if (PB->targetSize < 0)
    {
      sprintf (err, "Err. param. targetSize value: %d", PB->targetSize);
    }
  /*else if ((PB->IOCount * PB->IOSize * abs (PB->order) + PB->IOShift > PB->targetSize) && (PB->microBenchID != 4) && (PB->microBenchID != 3))
    {
      sprintf (err, "targetSize is too small value: %d", PB->targetSize);
    }
  else if (PB->targetSize + PB->targetOffset > PB->deviceSize)
    {
      sprintf (err, "targetSize + Offset > device Size value: %d", PB->deviceSize);
    }*/
  else if (PB->targetOffset < 0)
    {
      sprintf (err, "Err. param. targetOffset value: %d", PB->targetOffset);
    }
  else if ((PB->nbPartition < 0) || (PB->nbPartition > PB->IOCount))
    {
      sprintf (err, "Err. param. nbPartition is negative or smaller than IOCOunt value: %d", PB->nbPartition);
    }
  else if (PB->nbPartition > MAX_PARTITIONS)
    {
      sprintf (err, "Err. param. nbPartition is greater than MAX_PARTITIONS value: %d", PB->nbPartition);
    }
  else if ((PB->IOShift < 0) || (PB->IOShift > PB->IOSize))
    {
      sprintf (err, "Err. param. IOShift value: %d", PB->IOShift);
    }
  else if (PB->ignoreIO < 0)
    {
      sprintf (err, "Err. param. ignoreIO value: %d", PB->ignoreIO);
    }
  else if (PB->pauseIO < 0)
    {
      sprintf (err, "Err. param. pauseIO value: %d", PB->pauseIO);
    }
  else if (PB->pauseBurst < 0)
    {
      sprintf (err, "Err. param. pauseBurst value: %d", PB->pauseBurst);
    }
  else if ((PB->burstIO < 0) || (PB->burstIO > PB->IOCount))
    {
      sprintf (err, "Err. param. burstIO value: %d", PB->burstIO);
    }
  else if ((PB->parDeg < 0) || (PB->parDeg > MAX_PARALLEL))
    {
      sprintf (err, "Err. param. parDeg value: %d", PB->parDeg);
    }
  else if ((PB->processID < 0) || (PB->processID > PB->parDeg))
    {
      sprintf (err, "Err. param. processID value: %d", PB->processID);
    }
  else if (PB->device == NULL)
    {
      sprintf (err, "%s", "Err. missing device.");
    }
  else if ((PB->collectErase != INT32_MAX) && ((PB->collectErase < 0) || (PB->collectErase > 512)))
    {
      sprintf (err, "Err. param. collectErase value: %d", PB->collectErase);
    }
  else if (PB->pauseExp < 0)
    {
      sprintf (err, "Err. param. pauseExp value: %d", PB->pauseExp);
    }
  else
    {
      return NULL;
    }
  return strdup (err);
}



void
InitParams (UflipParams *PB)
{
  size_t      nameSize = MAX_STR;
  char        computerName [MAX_STR];
  time_t      lTime = 0;              /* variables used for time stamp generation */
  static char cpTime [1024] = "";

  strcpy (PB->expPlan, "");
  strcpy (PB->expSelect, "");
  strcpy (PB->outName, "");
  PB->nbRun         = 3;
  PB->IOSize        = 64;
  PB->IOCount       = 2048;
  PB->IOCountSR     = 128;
  PB->IOCountRR     = 256;
  PB->IOCountSW     = 1024;
  PB->IOCountRW     = 5116;
  PB->IOCount2      = 20480;
  PB->targetSize    = 0;
  PB->targetOffset  = 0;
  PB->targetSize2   = 0;
  PB->targetOffset2 = 0;
  PB->nbPartition   = 1;
  PB->IOShift       = 0;
  PB->ignoreIO      = 0;
  PB->ignoreIOSR    = 0;
  PB->ignoreIORR    = 0;
  PB->ignoreIOSW    = 0;
  PB->ignoreIORW    = 50;
  PB->ratio         = 0;
  PB->order         = 1;
  PB->pauseIO       = 0;
  PB->pauseBurst    = 0;
  PB->burstIO       = 1;
  PB->parDeg        = 0;
  PB->processID     = 0;
  strcpy (PB->comment, "");
  strcpy (PB->base, "SR");
  strcpy (PB->base2, "");
  time (&lTime);  /* get the current time */
  strcpy (cpTime, ctime (&lTime));
  cpTime [strlen (cpTime) - 1] = 0; /* ctime adds a trailing newline which we don't want */
  strcpy (PB->timeStamp, cpTime);
  if (uflip_gethostname (computerName, nameSize))
    strcpy (PB->machine,  computerName);

  /* don't create a device, it is created in get_common_*(). Otherwise we'll */
  /* create memleaks here (or later) */
  /*PB->device = uflip_device_new (NULL);*/
  PB->deviceSize   = 0;
  PB->collectErase = INT32_MAX;
  PB->pauseExp     = 100;
  PB->fake         = false;
  PB->bufferType   = NO_BUFFERING;
  PB->fmtType      = RND;
  PB->microBenchID = 0;
  PB->expID        = 0;
  PB->runID        = 0;
  PB->warning      = 0;
  PB->key          = 0;
}



/**
 * Sets its default value for each benchmark parameter for the RndFormat function.
 *
 * @param PB a pointer to a structure containing benchmark parameters for this session
 */
static void
InitParamsRF (UflipParams *PB)
{
  InitParams (PB);
  PB->microBenchID = 0; /* Not really a micro bench */
  PB->expID        = 6; /* Exp 1 to 5 (3 times) allow determining startup, period, iocount */
  PB->runID        = 1; /* Only one run ! */
  strcpy (PB->comment, "FOR");
}



/**
 * Sets its default value for each benchmark parameter for the SeqFormat function.
 *
 * @param PB a pointer to a structure containing benchmark parameters for this session
 */
static void
InitParamsSF (UflipParams *PB)
{
  InitParams (PB);
  PB->microBenchID = 0; /* Not really a micro bench */
  PB->expID        = 7; /* Exp 1 to 5 (3 times) allow determining startup, period, iocount */
  PB->runID        = 1; /* Only one run ! */
  strcpy (PB->comment, "SQF");
}



/**
 * Sets its default value for each benchmark parameter for the GenPrepare function.
 *
 * @param PB a pointer to a structure containing benchmark parameters for this session
 */
static void
InitParamsGP (UflipParams *PB)
{
  InitParams (PB);
  strcpy (PB->outName, "Prepare"SCRIPT_EXTENSION);
}



/**
 * Sets its default value for each benchmark parameter for the GenBench function.
 *
 * @param PB a pointer to a structure containing benchmark parameters for this session
 */
static void
InitParamsGB (UflipParams *PB)
{
  InitParams (PB);
  strcpy (PB->expPlan, "ExpPlan.txt");
  strcpy (PB->expSelect, "ExpSel.txt");
  strcpy (PB->outName, "Bench"SCRIPT_EXTENSION);
}



/**
 * Sets its default value for each benchmark parameter for the Run function.
 *
 * @param PB a pointer to a structure containing benchmark parameters for this session
 */
static void
InitParamsR (UflipParams *PB)
{
  InitParams (PB);
}



UflipParams
*GetTestParameters (unsigned int argc,
                    char       **argv)
{
  UflipParams *PB;
  /*char        str [MAX_STR];*/ /* Temporary string */

  PB = uflip_params_new ();
  if (PB == NULL)
    HandleError (__func__, "couldn't allocate Param block", 0, ERR_ABORT);

  /* don't create a device, it is created in get_common_*(). Otherwise we'll */
  /* create memleaks here (or later) */
  /*PB->device = uflip_device_new (NULL);
  if (PB->device == NULL)
        HandleError (__func__, "couldn't allocate Param->device subblock", 0, ERR_ABORT);*/

  if (argc < 2 || argc % 2 != 0)
    {
      uflip_params_destroy (PB);
      PB = NULL;
      PrintHelp ("");
    }

  /*sprintf (str, "running [%s] with function [%s]\n", argv[0], argv[1]);*/
  /*OutputString (OUT_LOG, "\n");*/
  /*OutputString (OUT_LOG, str);*/

  if (strcasecmp (argv [1], "RandomFormat") == 0)
    {
      PB->mainFunction = RND_FORMAT;
      InitParamsRF (PB);

      for (size_t i = 2; i < argc; i += 2)
        GetValuesRF (argv [i], argv [i + 1], PB);
    }
  else if (strcasecmp (argv [1], "SequentialFormat") == 0)
    {
      PB->mainFunction = SEQ_FORMAT;
      InitParamsSF (PB);

      for (size_t i = 2; i < argc; i += 2)
        GetValuesSF (argv [i], argv [i + 1], PB);
    }
  else if (strcasecmp (argv [1], "GenPrepare") == 0)
    {
      PB->mainFunction = GEN_PREPARE;
      InitParamsGP (PB);

      for (size_t i = 2; i < argc; i += 2)
        GetValuesGP (argv [i], argv [i + 1], PB);
    }
  else if (strcasecmp (argv [1], "GenBench") == 0)
    {
      PB->mainFunction = GEN_BENCH;
      InitParamsGB (PB);

      for (size_t i = 2; i < argc; i += 2)
        GetValuesGB (argv [i], argv [i + 1], PB);
    }
  else if (strcasecmp (argv [1], "Run") == 0)
    {
      PB->mainFunction = RUN_BENCH;
      InitParamsR (PB);

      for (size_t i = 2; i < argc; i += 2)
        {
          if (GetValues(argv [i], argv [i + 1], PB, NULL, NULL) == -1)
            HandleError (__func__, "Problem with the experimentation plan!", 0, ERR_ABORT);
        }

      char * err = CheckPB(PB);
      if (err != NULL)
        HandleError (__func__, err, 0, ERR_ABORT);
    }
  else
    {
      uflip_params_destroy (PB);
      PB = NULL;
      PrintHelp ("");
    }
  return (PB);
}



void
PrintHelp (const char *varName)
{
  size_t i;

  fprintf (stderr, "\na keyword was not recognized: %s\n", varName);

  fprintf (stderr, "Usage: "PACKAGE_NAME" <fonction> [options]\n\n"
                  "<fonction> may include one of the following:\n");
  for (i = 0; functions [i] [0] != NULL; ++i)
    fprintf (stderr, "  %s\t%s\n", functions [i] [0], functions [i] [1]);

  fprintf (stderr, "\nOptions common to all functions:\n");
  for (i = 0; common_options [i] [0] != NULL; ++i)
    fprintf (stderr, "  %s %s\t%s\n", common_options [i] [0], common_options [i] [1], common_options [i] [2]);

  fprintf (stderr, "\nFor the RandomFormat function, [options] may include any of the following:\n");
  for (i = 0; rnd_options [i] [0] != NULL; ++i)
    fprintf (stderr, "  %s %s\t%s\n", rnd_options [i] [0], rnd_options [i] [1], rnd_options [i] [2]);

  fprintf (stderr, "\nFor the SequentialFormat function, [options] may include any of the following:\n");
  for (i = 0; seq_options [i] [0] != NULL; ++i)
    fprintf (stderr, "  %s %s\t%s\n", seq_options [i] [0], seq_options [i] [1], seq_options [i] [2]);

  fprintf (stderr, "\nFor the GenPrepare function, [options] may include any of the following:\n");
  for (i = 0; genprep_options [i] [0] != NULL; ++i)
    fprintf (stderr, "  %s %s\t%s\n", genprep_options [i] [0], genprep_options [i] [1], genprep_options [i] [2]);

  fprintf (stderr, "\nFor the GenBench function, [options] may include any of the following:\n");
  for (i = 0; genbench_options [i] [0] != NULL; ++i)
    fprintf (stderr, "  %s %s\t%s\n", genbench_options [i] [0], genbench_options [i] [1], genbench_options [i] [2]);

  fprintf (stderr, "\nFor the Run function, [options] may include any of the following:\n");
  for (i = 0; run_options [i] [0] != NULL; ++i)
    fprintf (stderr, "  %s %s\t%s\n", run_options [i] [0], run_options [i] [1], run_options [i] [2]);

  exit (1);
}
