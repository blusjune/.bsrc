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

#include <stdlib.h>

#include "uflip_device.h"

UflipDevice
*uflip_device_new (const char *desired_value)
{
  UflipDevice *device = NULL;

  device = (UflipDevice *) malloc (sizeof (UflipDevice));
  if (device != NULL)
    {
      device->data = NULL;
      if (uflip_device_populate (device, desired_value) != 0)
        {
          free (device);
          return NULL;
        }
    }

  return device;
}



int
uflip_device_open (UflipDevice *device,
                  int          flags)
{
  if (device == NULL || device->data == NULL || device->open == NULL)
    return -1;

  return device->open (device, flags);
}



int
uflip_device_read (UflipDevice *device,
                   void        *buf,
                   size_t       count)
{
  if (device == NULL || device->data == NULL || device->read == NULL)
    return -1;

  return device->read (device, buf, count);
}



int
uflip_device_write (UflipDevice *device,
                    const void  *buf,
                    size_t       count)
{
  if (device == NULL || device->data == NULL || device->write == NULL)
    return -1;

  return device->write (device, buf, count);
}



int64_t
uflip_device_seek (UflipDevice *device,
                   int64_t      offset,
                   int          whence)
{
  if (device == NULL || device->data == NULL || device->seek == NULL)
    return -1;

  return device->seek (device, offset, whence);
}



int
uflip_device_close (UflipDevice *device)
{
  if (device == NULL || device->data == NULL || device->close == NULL)
    return -1;

  return device->close (device);
}



uint64_t
*uflip_device_get_geometry (UflipDevice *device)
{
  if (device == NULL || device->data == NULL || device->get_geometry == NULL)
    return NULL;

  return device->get_geometry (device);
}



int
uflip_device_get_erase_count (UflipDevice *device,
                              int32_t      address)
{
  if (device == NULL || device->data == NULL || device->get_erase_count == NULL)
    return 0;

  return device->get_erase_count (device, address);
}



const char
*uflip_device_to_string (UflipDevice *device)
{
  if (device == NULL || device->data == NULL || device->to_string == NULL)
    return NULL;

  return device->to_string (device);
}



void
uflip_device_destroy (UflipDevice *device)
{
  if (device == NULL)
    return;

  if (device->destroy != NULL)
    device->destroy (device);

  if (device->data != NULL)
    {
      free (device->data);
      device->data = NULL;
    }

  free (device);
}



UflipDevice
*uflip_device_copy (UflipDevice *device)
{
  return (uflip_device_new (uflip_device_to_string (device)));
}
