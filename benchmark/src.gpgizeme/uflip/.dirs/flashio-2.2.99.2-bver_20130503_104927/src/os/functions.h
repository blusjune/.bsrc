/**
 * @file functions.h
 * @brief Headers for os-dependent functions
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

#ifndef OS_FUNCTIONS_H_
#define OS_FUNCTIONS_H_

#include <string.h>

/**
 * Wrapper to malloc sector-aligned memory.
 *
 * @param [in,out] memptr address of the allocated memory
 * @param [in] size number of bytes to allocate
 * @return zero on success, non-zero otherwise.
 */
int  uflip_malloc_aligned (void **memptr,
                           size_t size);
/**
 * Wrapper to free sector-aligned memory.
 *
 * @param [in,out] ptr address of the memory to free
 */
void uflip_free_aligned   (void  *ptr);
/**
 * Wrapper around <tt>gethostname</tt>/<tt>GetComputerName</tt>.
 *
 * @param [out] name buffer to store the name
 * @param [in] namelen size of the buffer
 * @return 0 on success, -1 on failure
 */
int  uflip_gethostname    (char  *name,
                           size_t namelen);
/**
 * Wrapper around <em>errno</em>/<tt>GetLastError</tt>.
 *
 * @return the last error code
 */
int  get_last_error       (void);

#endif /* !OS_FUNCTIONS_H_ */
