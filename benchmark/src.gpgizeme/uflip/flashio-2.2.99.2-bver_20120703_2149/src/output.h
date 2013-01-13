/**
 * @file output.h
 * @brief Headers for the output module
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

#ifndef OUTPUT_H_
#define OUTPUT_H_

#include "uflip_params.h"
#include "uflip_results.h"

/** Max length for a file name */
#define MAX_FILENAME 400

/** Files identifiers */
enum file_ident {
  OUT_ERR          = 0, /**< Error file */
  OUT_TIM          = 1, /**< Timing file */
  OUT_RES          = 2, /**< Result file */
  OUT_TRA          = 3, /**< Trace file */
  OUT_ALL          = 4, /**< File containing all results in a bidimensional table */
  OUT_SOR          = 5, /**< Same as OUT_ALL but sorted by IO cost */
  OUT_TMP          = 6, /**< Temporary file (for sorting) */
  OUT_COL          = 7, /**< name of the column in the ALL and SOR files */
  OUT_AVG          = 8, /**< Same as OUT_ALL but presents the running average (excluding IgnoreIO) */
  OUT_LOG          = 9, /**< sequential log accross experiments */
  MAX_OUTPUT_FILES = 10 /**< Number of Output files */
};

/** Error levels for HandleError() */
enum err_lvl {
  ERR_SUCCESS, /**< used for non-error conditions such as progress indicators */
  ERR_ABORT    /**< indicates a critical error has occured */
};

/**
 * Initializes the logfile name to the given <tt>NULL</tt>-terminated string.
 *
 * @param str [in] logfile name
 */
void InitLogName   (const char   *str);
/**
 * Writes out the given <tt>NULL</tt>-terminated string to the output file corresponding to
 * the given file number.
 *
 * @param [in] numFile output file number
 * @param [in] str <tt>NULL</tt>-terminated string to write
 */
void OutputString  (int           numFile,
                    const char   *str);
/**
 * Initializes the output filenames to correct values.
 *
 * @param [in,out] PB a pointer to a structure containing benchmark parameters for this session
 */
void InitFileNames (UflipParams  *PB);
/**
 * Formats data contained within the two given structures and converts it into a single CSV line
 * which can easily be imported into a spreadsheet. It also completes (and sorts) the ALL and SOR
 * files.
 *
 * @param [in] PB a pointer to a structure containing benchmark parameters for this session
 * @param [in] RB a pointer to a structure containing benchmark results for this session
 */
void OutputResults (UflipParams  *PB,
                    UflipResults *RB);
/**
 * Provides error handling.
 *
 * @param [in] caller string containing the name of the function in which the error occurred
 * @param [in] error string containing the error message
 * @param [in] errNum error number
 * @param [in] errLevel severity of the error
 */
void HandleError   (const char   *caller,
                    const char   *error,
                    int           errNum,
                    enum err_lvl  errLevel);
/**
 * Writes the given <tt>NULL</tt>-terminated string to log files, and, additionally, outputs it to stderr.
 *
 * @param [in] buffer <tt>NULL</tt>-terminated string to output
 */
void OutputLine    (const char   *buffer);

#endif /* !OUTPUT_H_ */
