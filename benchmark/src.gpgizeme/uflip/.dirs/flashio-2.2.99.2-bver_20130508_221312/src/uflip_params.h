/**
 * @file uflip_params.h
 * @brief Headers for parameters in uFLIP
 */
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

#ifndef UFLIP_PARAMS_H_
#define UFLIP_PARAMS_H_

#include <stdbool.h>

#include "uflip_device.h"

/** max number of characters in common strings (should be refined) */
#define MAX_STR 1000

/** Possible formats */
enum fmt_opt
{
  NONE, /**< No format */
  SEQ,  /**< Sequential format */
  RND   /**< Random format */
};

/** Enumeration of buffering options */
enum buf_opt
{
  NO_BUFFERING = 0, /**< Neither HW nor SW buffering */
  FS_BUFFERING = 1, /**< Software Buffering */
  HW_BUFFERING = 2  /**< Hardware Buffering */
};

/** Main functions */
enum bench_func
{
  GEN_PREPARE, /**< Generate Preparation */
  RND_FORMAT,  /**< Random Format */
  SEQ_FORMAT,  /**< Sequential Format */
  GEN_BENCH,   /**< Generate Benchmark */
  RUN_BENCH    /**< Run Benchmark */
};

/** Input parameters */
typedef struct _UflipParams UflipParams;

/**
 * @brief Container used to represent input parameters
 * @see FlashIO's help output
 */
struct _UflipParams
{
  enum bench_func mainFunction;        /**< Function executed during this benchmark session */
  char            expPlan [MAX_STR];   /**< Filename containing the experimentation plan */
  char            expSelect [MAX_STR]; /**< Filename of a file containing the selection of bench to generate wrt to expPlan */
  char            outName [MAX_STR];   /**< Filename for generating the bench or the preparation */
  int32_t         nbRun;               /**< Number of run for each experiment */
  int32_t         IOSize;              /**< size in sectors (512B) of each IO */
  int32_t         IOCount;             /**< Total Nb of IOs */
  int32_t         IOCountSR;           /**< Total Nb of IOs for generating bench for SR */
  int32_t         IOCountRR;           /**< Total Nb of IOs for generating bench for RR */
  int32_t         IOCountSW;           /**< Total Nb of IOs for generating bench for RR */
  int32_t         IOCountRW;           /**< Total Nb of IOs for generating bench for RW */
  int32_t         IOCount2;            /**< Total Nb of IOs used only for determining startup/period */
  int32_t         targetSize;          /**< Size in sectors of the space where test will be performed */
  int32_t         targetOffset;        /**< offset in sectors to perform tests */
  int32_t         targetSize2;         /**< Size in sectors of the space where test will be performed */
  int32_t         targetOffset2;       /**< offset in sectors to perform tests */
  int32_t         nbPartition;         /**< For partitionned patterns (P) */
  int32_t         IOShift;             /**< Small shift for testing unaligned IOs. in sectors. */
  int32_t         ignoreIO;            /**< Number of IOs to ignore when computing stats. */
  int32_t         ratio;               /**< ration of pattern 1 / pattern 2 (if negative it is the contrary) */
  int32_t         order;               /**< For ordered patterns, linear increase or decrease (in blocks) */
  int32_t         pauseIO;             /**< pause between IOs (in microseconds) */
  int32_t         pauseBurst;          /**< Pause between burst of B IOs (in microseconds) */
  int32_t         burstIO;             /**< Number of IOs in a burst (B) */
  int32_t         parDeg;              /**< Parallel degree */
  int             processID;           /**< Process ID between 1 and 16 when parallel IOs */
  char            comment [MAX_STR];   /**< user defined string containing information about the current test. No commas allowed. */
  char            timeStamp [MAX_STR]; /**< string containing system generated time and date stamp */
  char            machine [MAX_STR];   /**< machine name of system under test */
  UflipDevice    *device;              /**< Device */
  int32_t         deviceSize;          /**< size of the device, in sectors */
  int32_t         collectErase;        /**< indicate if Erase count (SMART) should be collected and its address */
  int             pauseExp;            /**< pause between exps (in ms) */
  bool            fake;                /**< Run fake tests (no real IOs) - To check the arguments */
  int             bufferType;          /**< enumerated buffering option */
  enum fmt_opt    fmtType;             /**< Format type for GenPrepare */
  int             microBenchID;        /**< Bench ID between 0 and 9 (0 = prepare) */
  int             expID;               /**< Experiment ID between 1 and 5 */
  int             runID;               /**< RunID between 1 and n (depending on the number of values) */
  int             warning;             /**< Warning messageID - can be computed during the bench generation */
  int             key;                 /**< a unique key to identify sequentially the experiment */
  int             ignoreIOSR;          /**< Number of IOs to ignore when computing stats for generating bench for SR. */
  int             ignoreIORR;          /**< Number of IOs to ignore when computing stats for generating bench for RR. */
  int             ignoreIOSW;          /**< Number of IOs to ignore when computing stats for generating bench for SW. */
  int             ignoreIORW;          /**< Number of IOs to ignore when computing stats for generating bench for RW. */
  char            base [3];            /**< SR, RR, SW, RW for generating the bench */
  char            base2 [3];           /**< SR, RR, SW, RW for generating MIX bench */
};

/**
 * Creates new uFLIP parameters.
 *
 * As its size is only known at runtime, depending on the platform, the @a device
 * pointer is set to @c NULL. It is the responsibility of the caller to call
 * uflip_device_new() or uflip_device_copy() before use.
 *
 * @returns @c NULL on failure, or a pointer to the newly created parameters on success.
 * It is the responsibility of the caller to call uflip_params_destroy() to free
 * it after use.
 */
UflipParams *uflip_params_new     (void);
/**
 * Destroys given params.
 *
 * @param [in,out] params params to destroy
 */
void         uflip_params_destroy (UflipParams *params);

#endif /* !UFLIP_PARAMS_H_ */
