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

#ifndef __CPHVB_VEM_CLUSTER_H

#include <cphvb.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Initialize the VEM
 *
 * @return Error codes (CPHVB_SUCCESS)
 */
DLLEXPORT cphvb_error cphvb_vem_cluster_init(cphvb_component *self);


/* Shutdown the VEM, which include a instruction flush
 *
 * @return Error codes (CPHVB_SUCCESS)
 */
DLLEXPORT cphvb_error cphvb_vem_cluster_shutdown(void);


/* Execute a list of instructions (blocking, for the time being).
 * It is required that the VEM supports all instructions in the list.
 *
 * @instruction A list of instructions to execute
 * @return Error codes (CPHVB_SUCCESS)
 */
DLLEXPORT cphvb_error cphvb_vem_cluster_execute(cphvb_intp count,
                                   cphvb_instruction inst_list[]);

/* Registre a new user-defined function.
 *
 * @lib Name of the shared library e.g. libmyfunc.so
 *      When NULL the default library is used.
 * @fun Name of the function e.g. myfunc
 * @id Identifier for the new function. The bridge should set the
 *     initial value to Zero. (in/out-put)
 * @return Error codes (CPHVB_SUCCESS)
 */
DLLEXPORT cphvb_error cphvb_vem_cluster_reg_func(char *fun, cphvb_intp *id);

#ifdef __cplusplus
}
#endif

#endif
