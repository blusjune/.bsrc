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

#if !defined(_WIN32) && !defined(_WIN64)
#error "_WIN32 and _WIN64 are not defined, aborting."
#endif

#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <math.h>
#include <windows.h>
#include <winioctl.h>

#include "functions.h"
#include "uflip_device.h"
#include "uflip_timer.h"

struct _UflipDeviceData
{
  char  *str;
  int    num;
  HANDLE fd;
};

#define DEVICE_PREFIX "\\\\.\\PhysicalDrive"
#define DEFAULT_DEVICE_VALUE 1

/* Values for system call for retreiving smart attributes */
#define DFP_RECEIVE_DRIVE_DATA 0x7C088
#define SMART_READ_ATTRIBUTE_VALUES 0xD0
#define IDE_EXECUTE_SMART_FUNCTION 0xB0
#define SMART_CYL_LOW 0x4F
#define SMART_CYL_HI 0xC2

/* These ones do not seem to be declared in MinGW. */
#if !defined(_MSC_VER)
/* SENDCMDINPARAMS irDriveRegs' member structure */
typedef struct
{
  UCHAR bFeaturesReg;
  UCHAR bSectorCountReg;
  UCHAR bSectorNumberReg;
  UCHAR bCylLowReg;
  UCHAR bCylHighReg;
  UCHAR bDriveHeadReg;
  UCHAR bCommandReg;
  UCHAR bReserved;
}
IDEREGS;

/* IDE IO command structure */
typedef struct
{
  ULONG cBufferSize;
  IDEREGS irDriveRegs;
  UCHAR bDriveNumber;
  UCHAR bReserved[3];
  ULONG dwReserved[4];
  UCHAR bBuffer[1];
}
SENDCMDINPARAMS;

/* SENDCMDOUTPARAMS DStatus' member structure */
typedef struct
{
  UCHAR bDriverError;
  UCHAR bIDEError;
  UCHAR bReserved[2];
  ULONG dwReserved[2];
}
DRIVERSTATUS;

/* IDE IO reply structure */
typedef struct
{
  ULONG cBufferSize;
  DRIVERSTATUS DriverStatus;
  UCHAR bBuffer[512];
}
SENDCMDOUTPARAMS;
#endif /* !defined(_MSC_VER) */

/* callbacks */
static int         open_win32             (UflipDevice   *device,
                                           int            flags);
static int         read_win32             (UflipDevice   *device,
                                           void          *buf,
                                           size_t         count);
static int         write_win32            (UflipDevice   *device,
                                           const void    *buf,
                                           size_t         count);
static int64_t     seek_win32             (UflipDevice   *device,
                                           int64_t        offset,
                                           int            whence);
static int         close_win32            (UflipDevice   *device);
static uint64_t   *get_geometry_win32     (UflipDevice   *device);
static int         get_erase_count_win32  (UflipDevice   *device,
                                           int32_t        address);
static const char *to_string_win32        (UflipDevice   *device);
static void        destroy_win32          (UflipDevice   *device);

static void        InitTimer              (void);
static void        FileTimeToLargeInteger (FILETIME      *ft,
                                           LARGE_INTEGER *li);

/* Global ticks per second */
LARGE_INTEGER TicksPerSecond =
  {
    .QuadPart = 0
  };
/* Global ticks per microsecond */
double TicksPerMicrosecond = 0;

/****************   InitTimer()   ***********************/
/* InitTimer is an internal function which sets the number of ticks per second and ticks per ms. */
static void
InitTimer (void)
{
  QueryPerformanceFrequency (&TicksPerSecond);
  TicksPerMicrosecond = (double) TicksPerSecond.QuadPart / 1E6;
}



static void
FileTimeToLargeInteger (FILETIME      *ft,
                        LARGE_INTEGER *li)
{
  li->LowPart  = ft->dwLowDateTime;
  li->HighPart = ft->dwHighDateTime;
}



/****************   StartTimers()   ***********************/
int
uflip_timer_start (UflipTimer *timer)
{
  FILETIME      K;
  FILETIME      U;
  FILETIME      ignore;
  LARGE_INTEGER ClockTime;
  LARGE_INTEGER UserTime;
  LARGE_INTEGER KernelTime;

  if (!QueryPerformanceCounter (&ClockTime))
    {
      /* no performance counter available */
      return -1;
    }
  timer->clock_time = ClockTime.QuadPart;
  GetProcessTimes (GetCurrentProcess (), /* me */
                   &ignore,              /* when the process was created */
                   &ignore,              /* when the process was destroyed */
                   &K,                   /* time the process has spent in kernel mode */
                   &U);                  /* time the process has spent in user mode */

  FileTimeToLargeInteger (&U, &UserTime); /* copy int64 */
  FileTimeToLargeInteger (&K, &KernelTime); /* copy int64 */

  timer->user_time   = UserTime.QuadPart;
  timer->kernel_time = KernelTime.QuadPart;

  return 0;
}



/****************   GetElapsedTime()   ***********************/
int
uflip_timer_get_elapsed_time (UflipTimer *timer,
                              double     *clock_time,
                              double     *user_time,
                              double     *kernel_time)
{
  LARGE_INTEGER StopTime;
  LARGE_INTEGER StopTimeU;
  LARGE_INTEGER StopTimeK;
  FILETIME      K;
  FILETIME      U;
  FILETIME      ignore;

  /* if this is a first-time call, initialize the globals. */
  if (TicksPerSecond.QuadPart == 0)
    InitTimer ();

  /* Get the current tick count */
  if (!QueryPerformanceCounter (&StopTime))
    return -1;

  /* convert ticks to seconds. */
  *clock_time = ((double) StopTime.QuadPart - timer->clock_time) / TicksPerMicrosecond / 1e6;
  /* get user and kernel times */
  GetProcessTimes (GetCurrentProcess (), /* me */
                   &ignore,              /* when the process was created */
                   &ignore,              /* when the process was destroyed */
                   &K,                   /* time the process has spent in kernel mode */
                   &U);                  /* time the process has spent in user mode */

  FileTimeToLargeInteger (&U, &StopTimeU); /* copy int64 */
  FileTimeToLargeInteger (&K, &StopTimeK); /* copy int64 */
  /* do the math (converting to seconds), times are in 100 ns units */
  *user_time   = ((double) StopTimeU.QuadPart - timer->user_time) / 1e7;
  *kernel_time = ((double) StopTimeK.QuadPart - timer->kernel_time) / 1e7;

  return 0;
}



int
uflip_malloc_aligned (void **memptr,
                      size_t size)
{
  *memptr = VirtualAlloc (NULL, size, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);
  return (*memptr == NULL);
}



void
uflip_free_aligned (void *ptr)
{
  VirtualFree (ptr, 0, MEM_RELEASE);
}



int
uflip_gethostname (char  *name,
                   size_t namelen)
{
  return ((GetComputerName (name, (DWORD *) &namelen) != FALSE) ? 0 : -1);
}



int
get_last_error (void)
{
  return GetLastError ();
}



int
uflip_device_populate (UflipDevice *device,
                       const char  *desired_value)
{
  char name [256] = "";
  int  res;

  device->data = (UflipDeviceData *) malloc (sizeof (UflipDeviceData));
  if (device->data == NULL)
    return -1;

  device->data->fd = INVALID_HANDLE_VALUE;

  if (desired_value == NULL)
    {
      device->data->num = DEFAULT_DEVICE_VALUE;
    }
  else
    {
      res = sscanf (desired_value, "%d", &device->data->num);
      /* check for failure, better safe than sorry (e.g. I don't want to use another drive by accident) */
      if (res != 1)
        {
          free (device->data);
          return -1;
        }
    }

  /* used in some Windows functions */
  res = sprintf (name, DEVICE_PREFIX"%d", device->data->num);
  /* check also */
  if (res == -1)
    {
      free (device->data);
      return -1;
    }
  device->data->str = strdup (name);

  /* set callback functions */
  device->open = open_win32;
  device->read = read_win32;
  device->write = write_win32;
  device->seek = seek_win32;
  device->close = close_win32;
  device->get_geometry = get_geometry_win32;
  device->get_erase_count = get_erase_count_win32;
  device->to_string = to_string_win32;
  device->destroy = destroy_win32;

  return 0;
}



static int
open_win32 (UflipDevice *device,
            int          flags)
{
  DWORD fflags = 0; /* flags & attributes */

  fflags |= ((flags & NO_SYSTEM_BUFFER) ? FILE_FLAG_NO_BUFFERING : 0);
  fflags |= ((flags & NO_HD_CACHE) ? FILE_FLAG_WRITE_THROUGH : 0);

  device->data->fd = CreateFile (device->data->str,
                                 GENERIC_READ | GENERIC_WRITE,
                                 FILE_SHARE_READ | FILE_SHARE_WRITE,
                                 NULL,
                                 OPEN_EXISTING,
                                 fflags,
                                 NULL);

  return ((device->data->fd != INVALID_HANDLE_VALUE) ? 0 : -1);
}



static int
read_win32 (UflipDevice *device,
            void        *buf,
            size_t       count)
{
  DWORD read = 0;
  if (!ReadFile (device->data->fd, buf, count, &read, NULL))
    return -1;

  return read;
}



static int
write_win32 (UflipDevice *device,
             const void  *buf,
             size_t count)
{
  DWORD written = 0;
  if (!WriteFile (device->data->fd, buf, count, &written, NULL))
    return -1;

  return written;
}



static int64_t
seek_win32 (UflipDevice *device,
            int64_t      offset,
            int          whence)
{
  LARGE_INTEGER offsetn =
    {
      .QuadPart = offset
    };
  int fwhence;

  switch (whence)
    {
      case SEEK_CUR:
        fwhence = FILE_CURRENT;
        break;
      case SEEK_END:
        fwhence = FILE_END;
        break;
      case SEEK_SET:
      default:
        fwhence = FILE_BEGIN;
    }
  if (!SetFilePointerEx (device->data->fd, offsetn, &offsetn, fwhence))
    return -1;

  return offsetn.QuadPart;
}



static int
close_win32(UflipDevice *device)
{
  return (CloseHandle (device->data->fd) ? 0 : -1);
}



static uint64_t
*get_geometry_win32 (UflipDevice *device)
{
  HANDLE        fd = INVALID_HANDLE_VALUE; /* handle to the drive to be examined */
  BOOL          bResult;                   /* results flag */
  DWORD         junk;                      /* discard results */
  DISK_GEOMETRY geom;                      /* temporary storage */
  uint64_t     *dg = (uint64_t *) calloc (4, sizeof (uint64_t));

  if (!dg)
    return NULL;

  fd = CreateFile (device->data->str,                  /* drive */
                   0,                                  /* no access to the drive */
                   FILE_SHARE_READ | FILE_SHARE_WRITE, /* share mode */
                   NULL,                               /* default security attributes */
                   OPEN_EXISTING,                      /* disposition */
                   0,                                  /* file attributes */
                   NULL);                              /* do not copy file attributes */

  if (fd == INVALID_HANDLE_VALUE)
    {
      /* cannot open the drive */
      free (dg);
      return NULL;
    }

  bResult = DeviceIoControl (fd,                            /* device to be queried */
                             IOCTL_DISK_GET_DRIVE_GEOMETRY, /* operation to perform */
                             NULL, 0,                       /* no input buffer */
                             &geom, sizeof (geom),          /* output buffer */
                             &junk,                         /* # bytes returned */
                             (LPOVERLAPPED) NULL);          /* synchronous I/O */
  CloseHandle (fd);
  if (!bResult)
    {
      free (dg);
      return NULL;
    }

  dg [0] = geom.Cylinders.QuadPart;
  dg [1] = geom.TracksPerCylinder;
  dg [2] = geom.SectorsPerTrack;
  dg [3] = geom.BytesPerSector;

  return (dg);
}



static int
get_erase_count_win32 (UflipDevice *device,
                       int32_t      address)
{
  BOOL             bResult; /* results flag */
  DWORD            junk;    /* discard results */
  SENDCMDINPARAMS  pSCIP;
  SENDCMDOUTPARAMS pSCOP;

  /* necessary values for reading SMART attributes */
  /* bDriveNumber should not be used, see <http://msdn.microsoft.com/en-us/library/aa510304.aspx> */
  /*pSCIP.bDriveNumber = dev->num;*/                                       /* device number */
  pSCIP.cBufferSize = 512;                                                 /* buffer size for reading data */
  pSCIP.irDriveRegs.bDriveHeadReg = 0xA0 | ((device->data->num & 1) << 4);
  pSCIP.irDriveRegs.bFeaturesReg = SMART_READ_ATTRIBUTE_VALUES;            /* reading attributes */
  pSCIP.irDriveRegs.bCommandReg = IDE_EXECUTE_SMART_FUNCTION;              /* Execute a SMART Function */
  pSCIP.irDriveRegs.bSectorCountReg = 1;                                   /* 1 sector to read */
  pSCIP.irDriveRegs.bSectorNumberReg = 1;                                  /* Sector n° 1 */
  pSCIP.irDriveRegs.bCylHighReg = SMART_CYL_HI;                            /* Cylinder number HI */
  pSCIP.irDriveRegs.bCylLowReg = SMART_CYL_LOW;                            /* Cylinder number LOW */

  bResult = DeviceIoControl (device->data->fd,               /* device to be queried */
                             DFP_RECEIVE_DRIVE_DATA,         /* operation to perform */
                             (void*) &pSCIP, sizeof (pSCIP),
                             (void*) &pSCOP, sizeof (pSCOP),
                             &junk,                          /* # bytes returned */
                             (LPOVERLAPPED) NULL);           /* synchronous I/O */

  if (bResult)
    return(*((int *) &(pSCOP.bBuffer [address])));

  return 0;
}



static const char
*to_string_win32 (UflipDevice *device)
{
  /* we need a const char *, so extract the number (%d) from device->data->str */
  return (device->data->str + strlen (DEVICE_PREFIX));
}



static void
destroy_win32 (UflipDevice *device)
{
  if (device->data->fd != INVALID_HANDLE_VALUE)
    {
      close_win32 (device);
      device->data->fd = INVALID_HANDLE_VALUE;
      free (device->data->str);
      device->data->str = NULL;
    }
  device->data->num = -1;

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
