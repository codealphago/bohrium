/*
This file is part of Bohrium and copyright (c) 2012 the Bohrium
team <http://www.bh107.org>.

Bohrium is free software: you can redistribute it and/or modify
it under the terms of the GNU Lesser General Public License as 
published by the Free Software Foundation, either version 3 
of the License, or (at your option) any later version.

Bohrium is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the 
GNU Lesser General Public License along with Bohrium. 

If not, see <http://www.gnu.org/licenses/>.
*/
 
#ifndef CPHVB_VCACHE_H
#define CPHVB_VCACHE_H
#include <cphvb.h>

#ifdef __cplusplus
extern "C" {
#endif


/**
 *  Initiate vcache to a fixed size.
 *
 *  cache will hold 'size' elements in the cache.
 *
 *  Expiration / cache invalidation is based on round-robin;
 *  Whenever an element is added to vcache the round-robin
 *  counter is incremented.
 *
 */
DLLEXPORT void cphvb_vcache_init( int size );

/**
 * Deallocates vcache arrays.
 */
DLLEXPORT void cphvb_vcache_delete();

/**
 * Remove all entries from vcache and de-allocate them
 */
DLLEXPORT void cphvb_vcache_clear();

/**
 * Return and remove a pointer of size 'size' from vcache.
 *
 * This removes it from the vcache!
 *
 * @return null If none exists.
 */
DLLEXPORT cphvb_data_ptr cphvb_vcache_find( cphvb_intp bytes );

/**
 * Add an element to vcache.
 * 
 * @param data Pointer to allocated data.
 * @param size Size in bytes of the allocated data.
 */
DLLEXPORT void cphvb_vcache_insert( cphvb_data_ptr data, cphvb_intp size );
DLLEXPORT cphvb_error cphvb_vcache_free( cphvb_instruction* inst );
DLLEXPORT cphvb_error cphvb_vcache_malloc( cphvb_instruction* inst );

#ifdef __cplusplus
}
#endif

#endif /* CPHVB_VCACHE_H */
