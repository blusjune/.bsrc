/**
 * @file blocAlloc.h
 * @brief Headers for the blocAlloc module
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

#ifndef BLOCALLOC_H_
#define BLOCALLOC_H_

#include <stdint.h>

/** Free area on the device */
typedef struct _item item;

/** @brief Container used to represent the free area on the device */
struct _item {
  int32_t inf;  /**< start sector */
  int32_t sup;  /**< end sector */
  item   *prev; /**< pointer to the previous free area */
  item   *next; /**< pointer to the next free area */
};

/**
 * Initializes the list of free areas on the device.
 *
 * @param [in] size size of the free area (in blocks)
 * @return a free area on success, or @c NULL on failure
 */
item   *InitMemList           (int32_t size);
/**
 * Finds the first (i.e. lowest sector number) free area on the device.
 *
 * @param [in] list a pointer to the list of free areas on the device
 * @return the start sector of the first free area
 */
int32_t MemMinAddress         (item   *list);
/**
 * Finds a free area whose size is @a size on the device.
 *
 * @param [in] list a pointer to the list of free areas on the device
 * @param [in] size size of the area
 * @return the start sector of the free area found
 */
int32_t MemSearch             (item   *list,
                               int32_t size);
/**
 * Finds on the device a free area whose size is @a size and start is after @a A.
 *
 * @param [in] list a pointer to the list of free areas on the device
 * @param [in] A minimum start sector
 * @param [in] size size of the area
 * @return the start sector of the free area found, or -1 on failure (memory
 * exhausted)
 */
int32_t MemAllocNearestAfterA (item   *list,
                               int32_t A,
                               int32_t size);
/**
 * Finds on the device a free area containing A-B.
 *
 * @param [in] list a pointer to the list of free areas on the device
 * @param [in] A a sector
 * @param [in] B a sector
 * @return the start sector of the free area found, or -1 on failure (memory
 * exhausted)
 */
int32_t MemAlloc              (item   *list,
                               int32_t A,
                               int32_t B);

#endif /* !BLOCALLOC_H_ */
