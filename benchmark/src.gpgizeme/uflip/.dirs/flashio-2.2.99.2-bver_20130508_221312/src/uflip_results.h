/**
 * @file uflip_results.h
 * @brief Headers for results in uFLIP
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

#ifndef UFLIP_RESULTS_H_
#define UFLIP_RESULTS_H_

#include <stdbool.h>

/** Output results */
typedef struct _UflipResults UflipResults;

/**
 * @brief Container used to represent output results
 *
 * It is passed to all benchmark functions to store the gathered results.
 */
struct _UflipResults
{
  double  lostTime;   /**< time lost between IOs */
  double  timeClock;  /**< measured clock time in seconds */
  double  timeUser;   /**< measured user-mode processor time in milliseconds (ms) */
  double  timeKernel; /**< measured kernel-mode processor time in milliseconds (ms) */
  double  minIO;      /**< measured clock time in seconds for the cheapest IO (not in place) */
  double  maxIO;      /**< measured clock time in seconds for the more expensive IO (not in place) */
  double  avgIO;      /**< measured clock time in seconds for IO (average)(not in place) */
  double *timing;     /**< detailed timings */
  bool   *isWrite;    /**< detailed operationtype (true = WRITE) */
  bool   *isRND;      /**< detailed operation randomness (true = RANDOM) */
  int    *dPos;       /**< detailed block position (in block number) */
  double  stdDevIO;   /**< Standard deviation */
  int     initErase;  /**< Initial counter of erase */
  int     endErase;   /**< Final counter of erase */
  int     eraseCount; /**< Number of erase done during the test (for S.M.A.R.T SSD) */
  bool    success;    /**< status of the benchmark */
};

/**
 * Creates new uFLIP results.
 *
 * As their size is only known during the benchmark, the @a timing, @a isWrite,
 * @a isRND and @a dPos pointers are set to @c NULL. It is the responsibility of
 * the caller to malloc them before use.
 *
 * @returns @c NULL on failure, or a pointer to the newly created results on success.
 * It is the responsibility of the caller to call uflip_results_destroy() to free
 * it after use.
 */
UflipResults *uflip_results_new     (void);
/**
 * Destroys given results.
 *
 * @param [in,out] results results to destroy
 */
void          uflip_results_destroy (UflipResults *results);

#endif /* !UFLIP_RESULTS_H_ */
