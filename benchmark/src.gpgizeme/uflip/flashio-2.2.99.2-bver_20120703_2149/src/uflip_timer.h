/**
 * @file uflip_timer.h
 * @brief Headers for timers used in uFLIP
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

#ifndef UFLIP_TIMER_H_
#define UFLIP_TIMER_H_

/** Timer */
typedef struct _UflipTimer UflipTimer;

/**
 * @brief Container used to represent a timer
 *
 * It is passed to various timing functions which initialize the timer or
 * measure the elapsed time.
 */
struct _UflipTimer
{
  double clock_time;  /**< Wall clock time */
  double user_time;   /**< User mode time */
  double kernel_time; /**< Kernel mode time */
};

/**
 * Waits (actively) the given number of microseconds.
 *
 * @param [in] microseconds number of microseconds to wait
 * @return -1 on failure, 0 on success
 */
int         uflip_wait_microsec          (int         microseconds);
/**
 * Creates a timer.
 *
 * @return a new timer. It is the responsibility of the caller to call
 * uflip_timer_destroy() to free it after use.
 */
UflipTimer *uflip_timer_new              (void);
/**
 * Initializes a timer that can later be passed to uflip_timer_get_elapsed_time() to retrieve the
 * elapsed time on that particular timer. There are no limitations on the number of outstanding timers.
 *
 * @param [in,out] timer timer to start
 * @return -1 on failure, 0 on success
 */
int         uflip_timer_start            (UflipTimer *timer);
/**
 * Takes a timer initialized by uflip_timer_start() along with three pointers to doubles, and returns
 * the elapsed wall clock time along with the consumed user and kernel mode times.
 *
 * @param [in] timer the considered timer
 * @param [out] clock_time buffer to store the wall clock time
 * @param [out] user_time buffer to store the time spent in user mode
 * @param [out] kernel_time buffer to store the time spent in kernel mode
 * @return -1 on failure, 0 on success
 */
int         uflip_timer_get_elapsed_time (UflipTimer *timer,
                                          double     *clock_time,
                                          double     *user_time,
                                          double     *kernel_time);
/**
 * Destroys an uFLIP timer.
 *
 * @param [in,out] timer timer to destroy
 */
void        uflip_timer_destroy          (UflipTimer *timer);

#endif /* !UFLIP_TIMER_H_ */
