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

#include <stdio.h>
#include <stdlib.h>

#include "blocAlloc.h"
#include "uflip_device.h"

static void printMem (item *list);
static void CheckMem (item *list);

/**
 * Displays free areas' properties.
 *
 * @param list a pointer to the list to display
 */
static void
printMem (item *list)
{
  item *curr;

  printf ("===================\n");
  curr = list;
  while (curr != NULL)
    {
      printf ("%d %d %d (%d %d %d)\n", curr->inf / 100000, curr->sup / 100000, curr->sup / 100000 - curr->inf / 100000, curr->inf, curr->sup, curr->sup - curr->inf);
      curr = curr->next;
    }
  printf ("===================\n\n");
}



/**
 * Checks whether start sectors are still sector-aligned. Exits on failure.
 *
 * @param list a pointer to the list to check
 */
static void
CheckMem (item *list)
{
  item *curr;

  /*printMem (list);*/
  curr = list;
  while (curr != NULL)
    {
      if ((curr->inf % BLOCK) != 0)
        {
          printMem (list);
          printf ("Memory is corrupted (no more aligned) \n");
          exit (1);
        }
      curr = curr->next;
    }
}



item
*InitMemList (int32_t size)
{
  item *curr;

  curr = (item *) malloc (sizeof (item));
  if (curr == NULL)
    return NULL;

  curr->inf  = 0;
  curr->sup  = size;
  curr->prev = NULL;
  curr->next = NULL;

  return (curr);
}



int32_t
MemMinAddress (item *list)
{
  CheckMem (list);
  return (list->inf);
}



int32_t
MemSearch (item   *list,
           int32_t size)
{
  item   *curr    = NULL;
  item   *min     = NULL;
  int32_t minSize = INT32_MAX;

  CheckMem (list);
  if ((size % BLOCK) != 0)
    size = ((int32_t) (size / BLOCK)) * BLOCK + BLOCK;

  /* Find smallest space where size fits */
  curr = list;
  while (curr != NULL)
    {
      if ((curr->sup - curr->inf >= size) && (curr->sup - curr->inf < minSize))
        {
          minSize = curr->sup - curr->inf;
          min = curr;
        }
      curr = curr->next;
    }
  /* allocate */
  if (minSize == INT32_MAX)
    return (INT32_MAX);

  min->inf += size;
  /*printf ("[%d]\n",min->inf - size );*/
  CheckMem (list);
  return (min->inf - size);
}



int32_t
MemAllocNearestAfterA (item   *list,
                       int32_t A,
                       int32_t size)
{
  item   *curr;
  int32_t I;
  int32_t S;

  /*printf ("A: %d, size: %d\n", A,size);*/
  CheckMem (list);
  if ((size % BLOCK) != 0)
    size = ((int32_t) (size / BLOCK)) * BLOCK + BLOCK;

  if ((A % BLOCK) != 0)
    A = ((int32_t) (A / BLOCK)) * BLOCK + BLOCK;

  /* Find the first item after A or containing A */
  curr = list;
  while ((curr != NULL) && (curr->inf <= A) && (curr->sup <A))
    curr = curr->next;

  /* Find first space where size fits */
  I = curr->inf;
  S = curr->sup;
  while ((curr != NULL) && !(((I <= A) && (S > A) && (S - A >= size))) && !(((I > A) && (S - I > size))))
    {
      curr = curr->next;
      if (curr != NULL)
        {
          I = curr->inf;
          S = curr->sup;
        }
    }

  if (curr == NULL)
    return INT32_MAX;

  if ((I < A) && (S > A) && (S - A >= size))
    {
      item * newI;

      /* scinder */
      newI = (item *) malloc (sizeof (item));
      if (newI == NULL)
        return -1;

      newI->inf = A + size;
      newI->sup = S;
      curr->sup = A;
      newI->next = curr->next;
      newI->prev = curr;
      curr->next = newI;
      /*printMem (list);*/
      CheckMem (list);

      return A;
    }

  if ((I >= A) && (S - I > size))
    {
      curr->inf += size;
      /*printMem (list);*/
      CheckMem (list);
      return I;
    }

  CheckMem (list);

  return INT32_MAX;
}



int32_t
MemAlloc (item   *list,
          int32_t A,
          int32_t B)
{
  item   *curr;
  int32_t I;
  int32_t S;

  /*printf ("entering MemAlloc %d %d \n", A, B);*/
  /* Find the item containing A-B */
  CheckMem (list);
  curr = list;
  while ((curr != NULL) && ((curr->inf > A) || (curr->sup < B)))
    curr = curr->next;

  if (curr == NULL)
    return INT32_MAX;

  I = curr->inf;
  S = curr->sup;
  if ((I < A) && (B <= S))
    {
      item * newI;

      /* scinder */
      newI = (item *) malloc (sizeof (item));
      if (newI == NULL)
        return -1;

      newI->inf = B;
      newI->sup = S;
      curr->sup = A;
      newI->next = curr->next;
      newI->prev = curr;
      curr->next = newI;
      /*printMem (list);*/
      CheckMem (list);

      return A;
    }
  else if (I == A)
    {
      curr->inf = B;
      CheckMem (list);
      return A;
    }
  CheckMem (list);
  return INT32_MAX;
}
