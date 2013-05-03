/**
 * @file input.h
 * @brief Headers for the input module
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

#ifndef INPUT_H_
#define INPUT_H_

#include "uflip_params.h"

/** Maximum number of concurrent execution for PAR microbench */
#define MAX_PARALLEL 32
/** Max number of partitions in the PART bench */
#define MAX_PARTITIONS 256
/** Maximum number of values for a varying parameter */
#define MAX_VARYING_VAL 100

/**
 * Gets values associated to a given option for the Run function.
 *
 * @param [in] varName <tt>NULL</tt>-terminated string containing the option name
 * @param [in] varValue <tt>NULL</tt>-terminated string containing the option value
 * @param [in,out] PB a pointer to a structure containing benchmark parameters for this session
 * @param [in] valueList <tt>NULL</tt>-terminated string containing a list of values to parse (used only in genBench.cpp)
 * @param [in] tabVal an array where parsed values from valueList are stored (used only is genBench.cpp)
 * @return -1 if there is a problem with the experimentation plan, the number of
 * values found otherwise
 */
int32_t      GetValues         (const char  *varName,
                                const char  *varValue,
                                UflipParams *PB,
                                char        *valueList,
                                int32_t     *tabVal);
/**
 * Main loop which parses a given command line.
 *
 * @param [in] argc number of arguments
 * @param [in] argv list of arguments
 */
UflipParams *GetTestParameters (unsigned int argc,
                                char       **argv);
/**
 * Sets its default value for each benchmark parameter.
 *
 * @param [in,out] PB a pointer to a structure containing benchmark parameters for this session
 */
void         InitParams        (UflipParams *PB);
/**
 * Prints program usage information.
 *
 * @param [in] varName <tt>NULL</tt>-terminated string containing the name of the unrocognized variable
 */
void         PrintHelp         (const char  *varName);

#endif /* !INPUT_H_ */
