/* $XFree86: $ */
/*
 * file vloaduc.h
 *
 * loads microcode
 */

#ifndef _VLOADUC_H_
#define _VLOADUC_H_



/*
 * includes
 */

#include "vos.h"
#include "vtypes.h"



/*
 * defines 
 */

#define LITTLE_ENDIAN



/*
 * function prototypes
 */

int v_load_ucfile(struct v_board_t *board, char *file_name);



#endif /* #ifndef _VLOADUC_H_ */

/*
 * end of file vloaduc.h
 */
