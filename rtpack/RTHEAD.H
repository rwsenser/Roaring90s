/*
The MIT License (MIT)

Copyright (c) 2013 RW Senser

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
THE SOFTWARE.
*/


/* mugged to run rt_pack on PC, standalone */


/**********************************************************************
***********************************************************************
*                                                                     *
*         Module Exports (Component Wide)                             *
*                                                                     *
***********************************************************************
**********************************************************************/

#define BEGIN {
#define END }

#define Int int
#define Long long
#define Double double
#define Short short
#define Char char
#define Byte unsigned char
#define Void void

#define PROTOTYPING
#define I_ [
#define _I ]

#define EXPORT
#define IMPORT

#define OR      ||                /* logical or operator             */
#define AND     &&                /* logical and operator            */
#define MOD     %                 /* modulus division                */
#define BOR     |                 /* bit or operator                 */
#define BORAS   !=                /* bit or assignmnet operator      */
#define BAND    &                 /* bit and operator                */
#define BANDAS  &=                /* bit and assignment operator     */
#define BINV    ~                 /* bit inversion                   */
#define NOT     !                 /* logical not operator            */
#define NE      !=                /* logical not equal               */
#define EQ      ==                /* logical not equal               */

#define RT_ERROR_OFFSET  (-100)

#define RT_OK           (0)       /* success, happyness, joy     */

#define RT_ERROR        (-1 + RT_ERROR_OFFSET)
                                  /* I could just sh*t          */

#define RT_DIVIDE_ZERO  (-15 + RT_ERROR_OFFSET)
                                  /* division by 0              */

#define RT_GT           (1)       /* greater than               */

#define RT_EQ           (0)       /* equal                      */

#define RT_LT           (-1)      /* less than                  */


typedef struct
BEGIN
  char x;				// dummy field
END
  Rt_user_entry ;

typedef struct
BEGIN
  Byte  *value_ptr;               /* pointer to actual value    */
  Short length;                   /* length of field in bytes   */
  Byte  scale;                    /* scale (places below dp)    */
  Byte  format;                   /* field format               */
                                  /* FOR_SPCK, etc              */
  Byte  ro_flag;                  /* read only flag (ro != 0)   */
END
  Rt_stack_entry ;

#include "rtmexp.h"

#define REG_FETCH (2)


/**********************************************************************
***********************************************************************
*                                                                     *
*         End of Component Header File                                *
*                                                                     *
***********************************************************************
**********************************************************************/
