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

#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <sys/ioctl.h>
#include <sys/times.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>
#include <unistd.h>

#include "functions.h"
#include "output.h"
#include "uflip_timer.h"

struct _UflipDeviceData
{
  char *str;
  int   fd;
};

/* useful to print whereabouts */
#define DBG(out, msg) sprintf (out, "%s:%s:%d (%s)", __FILE__, __func__, __LINE__, msg)

/* used when 'Dev foo' wasn't specified on the cli */
#define DEFAULT_DEVICE_VALUE "/dev/null"

/* callbacks */
static int         open_dummy            (UflipDevice *device,
                                          int          flags);
static int         read_dummy            (UflipDevice *device,
                                          void        *buf,
                                          size_t       count);
static int         write_dummy           (UflipDevice *device,
                                          const void  *buf,
                                          size_t       count);
static int64_t     seek_dummy            (UflipDevice *device,
                                          int64_t      offset,
                                          int          whence);
static int         close_dummy           (UflipDevice *device);
static uint64_t   *get_geometry_dummy    (UflipDevice *device);
static int         get_erase_count_dummy (UflipDevice *device,
                                          int32_t      address);
static const char *to_string_dummy       (UflipDevice *device);
static void        destroy_dummy         (UflipDevice *device);

int
uflip_malloc_aligned (void **memptr,
                      size_t size)
{
  *memptr = malloc (size);
  return (*memptr == NULL);
}



void
uflip_free_aligned (void *ptr)
{
  free (ptr);
}



int
uflip_gethostname (char   *name,
                   size_t  namelen)
{
  return (gethostname (name, namelen) != -1);
}



int
get_last_error (void)
{
  return errno;
}



int
uflip_device_populate (UflipDevice *device,
                       const char  *desired_value)
{
  device->data = (UflipDeviceData *) malloc (sizeof (UflipDeviceData));
  if (device->data == NULL)
    return -1;

  device->data->fd = -1;

  if (desired_value == NULL)
    {
      device->data->str = strdup (DEFAULT_DEVICE_VALUE);
    }
  else
    {
      device->data->str = strdup (desired_value);
    }

  /* set callback functions */
  device->open = open_dummy;
  device->read = read_dummy;
  device->write = write_dummy;
  device->seek = seek_dummy;
  device->close = close_dummy;
  device->get_geometry = get_geometry_dummy;
  device->get_erase_count = get_erase_count_dummy;
  device->to_string = to_string_dummy;
  device->destroy = destroy_dummy;

  return 0;
}



static int
open_dummy (UflipDevice *device,
            int          flags)
{
  /* err msg */
  char context [100];

  DBG (context, "cache flags not implemented");
  HandleError (context, strerror (ENOSYS), ENOSYS, ERR_SUCCESS);

  /* synchronized read/write on the device by default */
  int fflags = O_RDWR | O_RSYNC | O_DSYNC | O_SYNC;

  device->data->fd = open (device->data->str, fflags);
  if (device->data->fd == -1)
    return -1;

  return device->data->fd;
}



static int
read_dummy (UflipDevice *device,
            void        *buf,
            size_t       count)
{
  return read (device->data->fd, buf, count);
}



static int
write_dummy (UflipDevice *device,
             const void  *buf,
             size_t       count)
{
  return write (device->data->fd, buf, count);
}



static int64_t
seek_dummy (UflipDevice *device,
            int64_t      offset,
            int          whence)
{
  return lseek (device->data->fd, offset, whence);
}



static int
close_dummy (UflipDevice *device)
{
  return close (device->data->fd);
}



static uint64_t
*get_geometry_dummy (UflipDevice *device)
{
  uint64_t *dg;
  /* err msg */
  char context [100];

  DBG (context, "not implemented");
  HandleError (context, strerror (ENOSYS), ENOSYS, ERR_SUCCESS);

  dg = (uint64_t *) calloc (4, sizeof (uint64_t));

  if (dg == NULL)
    return NULL;

  dg [0] = 125;
  dg [1] = 255;
  dg [2] = 63;
  dg [3] = 512;

  return (dg);
}



static int
get_erase_count_dummy (UflipDevice *device,
                       int32_t      address)
{
  /* err msg */
  char context [100];

  DBG (context, "not implemented");
  HandleError (context, strerror (ENOSYS), ENOSYS, ERR_SUCCESS);

  return 0;
}



static const char
*to_string_dummy (UflipDevice *device)
{
  return device->data->str;
}



static void
destroy_dummy (UflipDevice *device)
{
  if (device->data->fd != -1)
    {
      close_dummy (device);
      device->data->fd = -1;
      free (device->data->str);
      device->data->str = NULL;
    }

  /* prevent further access */
  device->open = NULL;
  device->read = NULL;
  device->write = NULL;
  device->seek = NULL;
  device->close = NULL;
  device->get_geometry = NULL;
  device->get_erase_count = NULL;
  device->to_string = NULL;
  device->destroy = NULL;
}
