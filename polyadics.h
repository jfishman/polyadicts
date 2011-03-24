
/* 
** This file is part of PerTwisted, a persistant storage framework
** tailored around the Twisted asynchronous network programming framework,
** utilizing the Berkeley Database, Cononical's Storm ORM, implementations
** of various DHTs and hopefully much more.
**
** Pertwisted is free software: you can redistribute it and/or modify
** it under the terms of the GNU General Public License as published by
** the Free Software Foundation, either version 3 of the License, or
** (at your option) any later version.
**
** PerTwisted is distributed in the hope that it will be useful,
** but WITHOUT ANY WARRANTY; without even the implied warranty of
** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
** GNU General Public License for more details.
**
** You should have received a copy of the GNU General Public License
** and the GNU Lesser Public License along with PerTwisted.  If not, see
** <http://www.gnu.org/licenses/>.
*/

#ifndef _polyad_h_DEFINED
#define _polyad_h_DEFINED

#include <stdbool.h>
#include <sys/types.h>
#include "varint.h"

typedef unsigned int polyad_len_t;

/**
 * polyid - an n-tuple of varint-packed unsigned integers
 */

typedef struct polyid_st
{
    // tuple-length 'n' and decoded unsigned 64-bit integers
    polyad_len_t n;
    uint64_t *values;
    // data length and backing buffer
    size_t size;
    void *data;
    // do we own the buffer
    bool shared;
} polyid_t;

polyid_t* polyid_new(polyad_len_t n, uint64_t *values);
polyid_t* polyid_load(void *data, bool shared, size_t maxlen);
void polyid_free(polyid_t *pack);

/**
 * polyad - an n-tuple of binary data segments with a polyid header
 */

typedef struct polyad_info_st
{
    // length of data pointed to by void* data
    size_t size;
    // memory region for data;
    void* data;
    // do we own the memory, or is it shared?
    bool shared;
} polyad_info_t;

typedef struct polyad_st
{
    // total polyad length and data pointer
    polyad_info_t self;
    // number and array of stored items
    polyad_len_t nitem;
    polyad_info_t *item;
} polyad_t;

/* read a polyad structure from a suppplied data pointer */
polyad_t* polyad_load(size_t size, void *data, bool shared);
/* free any non-shared memory associated with a polyad */
void polyad_free(polyad_t *polyad);

/**
 * The following three functions should be used in consecutive order only
 **/
/* initialize a new polyad object prepared to store n entries */
polyad_t* polyad_prepare(polyad_len_t nitem);
/* set the i'th item in a polyad to the given data */
int polyad_set(polyad_t *polyad, polyad_len_t i, size_t size, void *data, bool shared);
/* allocate a single memory buffer and store the packed items */
int polyad_finish(polyad_t *polyad);

#endif
