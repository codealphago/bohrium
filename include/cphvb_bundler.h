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

#ifndef __CPHVB_BUNDLE_H
#define __CPHVB_BUNDLE_H

#ifdef __cplusplus
extern "C" {
#endif

/** Calculates the bundleable instructions.
 *
 * @param inst The instruction list
 * @param start of the bundle
 * @param end of the bundle
 * @param base_max TODO: describe
 * @return Number of consecutive bundeable instruction
 */
cphvb_intp cphvb_inst_bundle(cphvb_instruction* insts, cphvb_intp start, cphvb_intp end, cphvb_intp base_max);

#ifdef __cplusplus
}
#endif

#endif
