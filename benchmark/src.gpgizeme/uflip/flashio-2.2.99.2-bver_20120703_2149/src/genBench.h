/**
 * @file genBench.h
 * @brief Headers for the genBench module
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

#ifndef GENBENCH_H_
#define GENBENCH_H_

#include "uflip_params.h"

/* should probably be replaced by an enum */
/** Granularity micro-bench */
#define GRA 1
/** Alignment micro-bench */
#define ALI 2
/** Locality micro-bench */
#define LOC 3
/** Partitioning micro-bench */
#define PAT 4
/** Order micro-bench */
#define ORD 5
/** Parallelism micro-bench */
#define PAR 6
/** Mix micro-bench */
#define MIX 7
/** Pause micro-bench */
#define PIO 8
/** Bursts micro-bench */
#define PBU 9

/**
 * Generates a batch named Bench.bat (default), which contains the complete benchmark.
 *
 * @param [in] PB a pointer to a structure containing benchmark parameters for this session
 */
void GenBench   (UflipParams *PB);
/**
 * Generates a batch named Prepare.bat (default), which contains 6 experiments:
 *   - Exp1: SPE - (Special) - runs IOCount SR then IOCount RW then IOCount SR
 *     The goal is here to determine the Pause value
 *   - Exp2: SIO.SR - runs IOCount2 SR - goal: determine IOIgnoreSR and IOCountSR
 *   - Exp3-5 : same for RR, SW, RW
 *   - Exp6 : Random format of the device.
 *
 * @param [in] PB a pointer to a structure containing benchmark parameters for this session
 */
void GenPrepare (UflipParams *PB);

#endif /* !GENBENCH_H_ */
