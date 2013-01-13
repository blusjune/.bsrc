/**
 * @file uflip_device.h
 * @brief Headers for devices in uFLIP
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

#ifndef UFLIP_DEVICE_H_
#define UFLIP_DEVICE_H_

#include <stdint.h>

/** Number of byte in a sector (smallest I/O unit) */
#define SECTOR 512
/** Number of sectors in a Block (for alignement of target offset) */
#define BLOCK 256

/** disables kernel cache */
#define NO_SYSTEM_BUFFER ((int) 1)
/** disables disk cache */
#define NO_HD_CACHE ((int) 2)

/** SSD device */
typedef struct _UflipDevice     UflipDevice;
/** Os-dependent device data */
typedef struct _UflipDeviceData UflipDeviceData;

/** @brief Container used to represent a device */
struct _UflipDevice
{
  /** private data */
  UflipDeviceData *data;

  /**
   * @brief callback to open a device
   * @see uflip_device_open()
   */
  int         (*open)            (UflipDevice *device,
                                  int          flags);
  /**
   * @brief callback to read bytes from a device
   * @see uflip_device_read()
   */
  int         (*read)            (UflipDevice *device,
                                  void        *buf,
                                  size_t       count);
  /**
   * @brief callback to write bytes to a device
   * @see uflip_device_write()
   */
  int         (*write)           (UflipDevice *device,
                                  const void  *buf,
                                  size_t       count);
  /**
   * @brief callback to reposition read/write device offset
   * @see uflip_device_seek()
   */
  int64_t     (*seek)            (UflipDevice *device,
                                  int64_t      offset,
                                  int          whence);
  /**
   * @brief callback to close a device
   * @see uflip_device_close()
   */
  int         (*close)           (UflipDevice *device);
  /**
   * @brief callback to get the device geometry (C/H/S)
   * @see uflip_device_get_geometry()
   */
  uint64_t   *(*get_geometry)    (UflipDevice *device);
  /**
   * @brief callback to get the device erase count (vendor specific)
   * @see uflip_device_get_erase_count()
   */
  int         (*get_erase_count) (UflipDevice *device,
                                  int32_t      address);
  /**
   * @brief callback to get the device unique string identifier
   * @see uflip_device_to_string()
   */
  const char *(*to_string)       (UflipDevice *device);
  /**
   * @brief callback to destroy a device
   * @see uflip_device_destroy()
   */
  void        (*destroy)         (UflipDevice *device);
};

/**
 * Initializes an uflip_device structure.
 *
 * @param [in,out] device pointer to the device
 * @param [in] desired_value device value (name or num, depending on the implementation)
 * @return 0 on success. Non-zero means failure.
 */
int          uflip_device_populate        (UflipDevice *device,
                                           const char  *desired_value);
/**
 * Creates a new uFLIP device.
 *
 * @param desired_value value used to construct the device
 * @return @c NULL on failure, or a pointer to the newly created device on success.
 * It is the responsibility of the caller to call uflip_device_destroy() to free
 * it after use.
 */
UflipDevice *uflip_device_new             (const char  *desired_value);
/**
 * Opens an uFLIP device.
 *
 * @param [in] device device to open
 * @param [in] flags extra parameters
 * @return -1 on failure, zero or a positive integer on success
 */
int          uflip_device_open            (UflipDevice *device,
                                           int          flags);
/**
 * Reads an uFLIP device.
 *
 * @param [in] device device to read
 * @param [out] buf buffer to store read data
 * @param [in] count number or bytes to read
 * @return -1 on failure, the number of bytes read on success
 */
int          uflip_device_read            (UflipDevice *device,
                                           void        *buf,
                                           size_t       count);
/**
 * Writes to an uFLIP device.
 *
 * @param [in] device device to read
 * @param [in] buf buffer containing data to write
 * @param [in] count number of bytes to write
 * @return -1 on failure, the number of bytes written on success
 */
int          uflip_device_write           (UflipDevice *device,
                                           const void  *buf,
                                           size_t       count);
/**
 * Repositions device offset.
 *
 * @param [in] device device to seek
 * @param [in] offset where to move the pointer
 * @param [in] whence position directive
 * @see @c SEEK_SET, @c SEEK_CUR and @c SEEK_END from @em stdio.h
 * @return -1 on failure, the new pointer position on success
 */
int64_t      uflip_device_seek            (UflipDevice *device,
                                           int64_t      offset,
                                           int          whence);
/**
 * Closes an uFLIP device.
 *
 * @param [in] device device to close
 * @return -1 on failure, zero or a positive integer on success
 */
int          uflip_device_close           (UflipDevice *device);
/**
 * Fetches the drive geometry.
 *
 * @param [in] device device to query
 * @return @c NULL on failure, or an array of 64-bits integers on success
 * (@c [0]: number of cylinders, @c [1] number of tracks per cylinder, @c [2] number of
 * sectors per track, @c [3] number of bytes per sector)
 */
uint64_t    *uflip_device_get_geometry    (UflipDevice *device);
/**
 * Fetches the erase count of a Flash cell.
 *
 * @param [in] device device to query
 * @param [in] address address to query in the device (vendor-specific)
 * @return 0 on failure, the erase count on success
 */
int          uflip_device_get_erase_count (UflipDevice *device,
                                           int32_t      address);
/**
 * Produces a printable string identifying the device.
 *
 * The returned string must identify uniquely the device and be usable by
 * uflip_device_new() to recreate an identical device.
 *
 * @param [in] device device to use
 * @return @c NULL on failure, or a string usable by @c printf on success.
 */
const char  *uflip_device_to_string       (UflipDevice *device);
/**
 * Destroys an uFLIP device.
 *
 * @param [in,out] device device to destroy
 */
void         uflip_device_destroy         (UflipDevice *device);
/**
 * Copies an existing device.
 *
 * @param [in] device device to copy
 * @return @c NULL on failure, or a new device on success. It is the
 * responsibility of the caller to call uflip_device_destroy() to free it after
 * use.
 */
UflipDevice *uflip_device_copy            (UflipDevice *device);

#endif /* !UFLIP_DEVICE_H_ */
