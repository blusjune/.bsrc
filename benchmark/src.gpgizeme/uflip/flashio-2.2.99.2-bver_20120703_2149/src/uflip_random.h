/**
 * @file uflip_random.h
 * @brief Headers for the random number generator
 * @author Agner Fog
 *
 * This file contains class declarations and other definitions for the uniform
 * random number generator.
 * This is a multiply-with-carry type of random number generator
 * invented by George Marsaglia.  The algorithm is:
 *
 * <tt>S = 2111111111*X[n-4] + 1492*X[n-3] + 1776*X[n-2] + 5115*X[n-1] + C</tt>@n
 * <tt>X[n] = S modulo 2^32</tt>@n
 * <tt>C = floor(S / 2^32)</tt>
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

 © 1997 - 2007 Agner Fog. All software in this library is published under the
 GNU General Public License with the further restriction that it cannot be
 used for gambling applications. See licence.htm
 © 2008-2010, Luc Bouganim <luc.bouganim@inria.fr>
 © 2009-2010, Lionel Le Folgoc <lionel.le_folgoc@inria.fr>
*/

#ifndef UFLIP_RANDOM_H_
#define UFLIP_RANDOM_H_

#include <stdint.h>

/** Random number generator */
typedef struct _UflipRandom UflipRandom;

/**
 * @brief Container used to represent a random number generator
 *
 * Random number generator of type Mother-of-All (Multiply with carry)
 */
struct _UflipRandom
{
  uint32_t x [5]; /**< History buffer */
};

/**
 * Creates a new random number generator.
 *
 * @param [in] seed seed to use
 * @return @c NULL on failure, or a pointer to the newly created generator on success.
 * It is the responsibility of the caller to call uflip_random_destroy() to free
 * it after use.
*/
UflipRandom *uflip_random_new      (uint32_t seed);
/**
 * Re-initializes the random number generator with a new seed.
 *
 * @param [in,out] rnd generator to reseed
 * @param [in] seed New seed to use
 */
void         uflip_random_init     (UflipRandom *rnd,
                                    uint32_t     seed);
/**
 * Gives an integer random number in the interval min <= x <= max.
 * (max-min < @c MAXINT).
 *
 * The precision is 2^-32 (defined as the difference in frequency between
 * possible output values). The frequencies are exact if max-min+1 is a
 * power of 2.
 *
 * @param [in] rnd generator to use
 * @param [in] min lower value
 * @param [in] max higher value
 * @return an integer random number
 */
int          uflip_random_get_int  (UflipRandom *rnd,
                                    int          min,
                                    int          max);
/**
 * Gives a floating point random number in the interval 0 <= x < 1. The
 * resolution is 32 bits.
 *
 * @param [in] rnd generator to use
 * @return a random floating point number
 */
double       uflip_random_get_dbl  (UflipRandom *rnd);
/**
 * Gives 32 random bits.
 *
 * @param [in] rnd generator to use
 * @return 32 random bits.
 */
uint32_t     uflip_random_get_bits (UflipRandom *rnd);
/**
 * Destroys given random number generator.
 *
 * @param [in,out] rnd random number generator to destroy
 */
void         uflip_random_destroy  (UflipRandom *rnd);

#endif /* !UFLIP_RANDOM_H_ */
