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
/**********************************************************************
***********************************************************************
*                                                                     *
*           MODULE NAME : Packed Math support in C. (rtpack.c)        *
*                                                                     *
*          REVISION NO. :                                             *
*                                                                     *
*                AUTHOR : R. W. Senser                                *
*                                                                     *
*                  DATE : 11/16/89 (Reworked 15.05.91)                *
*                                                                     *
*           DESCRIPTION : This module supplies functions to do        *
*                         roughly the following 370 assembler         *
*                         operations:                                 *
*                                                                     *
*                           entry point: rt_pckf                      *
*                           pack - display -> packed       (rt_pack). *
*                           unpk - packed -> display       (rt_unpk). *
*                           cvb  - packed -> int           (rt_cvbp). *
*                           cvd  - int    -> packed        (rt_cvdp). *
*                           zap  - packed -> packed        (rt_zapp). *
*                           srp  - shift rounded           (rt_srpp). *
*                           cp   - compare packed          (rt_cpp).  *
*                           ap, mp                                    *
*                           sp, dp - packed math           (rt_mthp). *
*                                                                     *
*                         These functions should be able to           *
*                         correctly handle packed math in any C       *
*                         environment.  These are based on the 370    *
*                         assembler operations that work with         *
*                         packed math.                                *
*                                                                     *
*               These routines are:                                   *
*                                                                     *
*       SSSSSSSS    LL          OOOOOOOO     WW     WW      WW        *
*       SS          LL          OO    OO     WW     WW      WW        *
*       SS          LL          OO    OO      WW   WW WW   WW         *
*       SSSSSSSS    LL          OO    OO      WW   WW WW   WW         *
*             SS    LL          OO    OO       WW WW   WW WW          *
*             SS    LL          OO    OO       WW WW   WW WW          *
*       SSSSSSSS    LLLLLLLL    OOOOOOOO        WWW     WWW           *
*                                                                     *
*                but they are correct! (I think?)                     *
*                                                                     *
*          TEST HISTORY :                                             *
*                                                                     *
*              MODIFIED :                                             *
*                         30.11.90 RW Senser                          *
*                         Added switch so that Warning only prints    *
*                         one time.                                   *
*                                                                     *
*                         15.05.91 RW  Senser                         *
*                         Reworked to work correctly in C only        *
*                         environment.  Removed warning message.      *
*                         1) added rt_cvap(), rt_cvpa(), rt_adda(),   *
*                            rt_suba(), rt_mula(), rt_diva().         *
*                         2) changed rt_srpp to do real shifts        *
*                         3) removed rt_cvbp() and rt_cxdp()          *
*                         4) added rt_adda(), rt_suba(), rt_mula(),   *
*                            and rt_diva().                           *
*                         5) setup to run with 31 digits!             *
***********************************************************************
**********************************************************************/

#include "rthead.h"

#define RT_PACK_ERROR (-69)   /* bogus error for packed errors! */

			      /* Be careful if you change this  */
			      /* next macro -- the last guy that*/
			      /* did lost his hair!             */
#define BYTE_ARRAY_SIZE (32)  /* size of byte arrays            */

#define Signed_Char     signed char

#define RT_POS   (1)          /* positive sign                  */
#define RT_NEG   (-1)         /* negative sign                  */

			      /* TRACE_DIV should always be     */
			      /* #undef unless you wish to have */
			      /* the divide routine use the     */
			      /* printf() function to write     */
			      /* intermediate results to the    */
			      /* stdout file.                   */
#undef TRACE_DIV
			      /* macro used by TRAVE_DIV only   */
#define SHOW_BA(x)        BEGIN \
			    int ss; \
			    int c;  \
			    for (ss=0; ss < BYTE_ARRAY_SIZE; ss++) \
			    BEGIN \
			      c = x I_ ss _I; \
			      printf("%d ", c); \
			    END \
			    printf("\n"); \
			  END

/**********************************************************************
*                                                                     *
*          module functions                                           *
*                                                                     *
**********************************************************************/

/**********************************************************************
*                                                                     *
*         FUNCTION NAME : Packed entry point routine. (rt_pckf)       *
*                                                                     *
*      INPUT PARAMETERS : packed block.                               *
*                                                                     *
*                                                                     *
*     OUTPUT PARAMETERS : N/A                                         *
*                                                                     *
*          RESPONSE CODE: RT_OK,                                      *
*                         div by 0,                                   *
*                                                                     *
*           DESCRIPTION : This is the common entry point for the      *
*                         packed routines.  This routine should be    *
*                         replaced by routines, for each platform,    *
*                         that correctly and rapidly process packed   *
*                         fields.  This version (hopefully) correctly *
*                         handles pack fields up to 15 digits.  It    *
*                         works entirely in C and simulates packed    *
*                         math and comparison.                        *
*                                                                     *
*      GLOBAL VARIABLES : N/A                                         *
*                                                                     *
*      FUNCTIONS CALLED : rt_pw10, rt_mthp, rt_cpp, rt_zapp, rt_cvbp, *
*                         rt_cvdp, rt_pack, rt_unpk.                  *
*                                                                     *
*                  DATE :                                             *
*                                                                     *
*              MODIFIED :                                             *
*                                                                     *
**********************************************************************/

Int rt_pckf (pack_ptr)

  Rt_pack_entry *pack_ptr;

BEGIN

  Int function;                   /* function code              */

  function = pack_ptr -> pck_func;

  switch ( (int) function)  /* this type cast to avoid errors when */
			    /* Int is set to something other than  */
			    /* int!                                */
  BEGIN
    case (RT_DEC_ADD):            /* packed addition            */
      return(rt_mthp(pack_ptr, '+'));
      break;
    case (RT_DEC_SUB):            /* packed subtraction         */
      return(rt_mthp(pack_ptr, '-'));
      break;
    case (RT_DEC_MUL):            /* packed multiply            */
      return(rt_mthp(pack_ptr, '*'));
      break;
    case (RT_DEC_DIV):            /* packed divide              */
      return(rt_mthp(pack_ptr, '/'));
      break;

    case (RT_DEC_CMP):            /* packed compare             */
      return(rt_cpp(pack_ptr));
      break;


    case (RT_DEC_PCK_PCK):        /* convert packed --> packed  */
      return(rt_zapp(pack_ptr));
      break;
    case (RT_DEC_PCK_DISP):       /* convert packed --> display */
      return(rt_unpk(pack_ptr));
      break;
    case (RT_DEC_DISP_PCK):       /* convert display --> packed */
      return(rt_pack(pack_ptr));
      break;

  END

  return(RT_ERROR);               /* if get here, it's an error */

END

/**********************************************************************
*                                                                     *
*         FUNCTION NAME : Pack a number. (rtpack)                     *
*                                                                     *
*      INPUT PARAMETERS : pack_block                                  *
*                                                                     *
*     OUTPUT PARAMETERS : N/A                                         *
*                                                                     *
*          RESPONSE CODE: RT_OK                                       *
*                         RT_ERROR                                    *
*                                                                     *
*           DESCRIPTION : Pack the number that is the "from1" parm    *
*                         and put it in the "to" parm. (see the       *
*                         pack_blk structure)                         *
*                                                                     *
*      GLOBAL VARIABLES : N/A                                         *
*                                                                     *
*      FUNCTIONS CALLED : rt_srpp.                                    *
*                                                                     *
*                  DATE :                                             *
*                                                                     *
*              MODIFIED :                                             *
*                                                                     *
**********************************************************************/

Int rt_pack (pack_ptr)

  Rt_pack_entry *pack_ptr;

  BEGIN

    Int sign;                     /* packed sign value          */
    Int hilo;                     /* switch for high/low nible  */
    Int move_size;                /* number of nibles to move   */
    Byte *move_ptr;               /* current work byte pointer  */
    Byte *chr_ptr;                /* current display digit      */
    Int chr;                      /* current character          */

    Int i;                        /* loop variable              */
    Int j;                        /* loop variable              */

    Int scale_dif;                /* difference in sclaes       */

    Int status;

    if (pack_ptr -> pck_from1_length > RT_SUNP_MAX OR
        pack_ptr -> pck_to_length > RT_SPCK_MAX)
      BEGIN                       /* wrong sized fields         */
        return(RT_PACK_ERROR);
      END

                                  /* handle the sign            */
    sign = RT_SPCK_POS;
    if (((* (Byte *)
       ((pack_ptr -> pck_from1_ptr) +
        (pack_ptr -> pck_from1_length) -1))
        BAND 0xf0) == RT_SUNP_NEG)
      BEGIN
        sign = RT_SPCK_NEG;
      END
                                  /* clear the to field         */
    memset(pack_ptr -> pck_to_ptr, 0x00,pack_ptr -> pck_to_length);

                                  /* find num nibles to move    */
    move_size = ((pack_ptr -> pck_to_length) * 2) - 1;
    if (move_size > (pack_ptr -> pck_from1_length))
      BEGIN
        move_size = (pack_ptr -> pck_from1_length);
      END

                                  /* set the sign nible         */
    move_ptr = (pack_ptr -> pck_to_ptr) +
               (pack_ptr -> pck_to_length) - 1;
    *((Byte *) move_ptr) = sign;

                                  /* set switch to hi byte      */
    hilo = 1;

                                  /* convert the rest           */
    for (i=0; i < move_size; i++)
      BEGIN
        j = (pack_ptr -> pck_from1_length) - 1 - i;
    /* The code from here to the comment line below was added to */
    /* get around an apparent bug in quick C.  With the line     */
    /* below only and with the debugger off, the wrong results   */
    /* are produced.  With the debugger on -- it worked??        */
        chr_ptr = (pack_ptr -> pck_from1_ptr);
        chr_ptr = chr_ptr + j;
        chr = *chr_ptr;
    /*    chr = *((from_ptr -> value_ptr) + j);   */
        if (hilo == 0)            /* do low nible               */
          BEGIN
            *((Byte *) (move_ptr)) = chr BAND 0x0f;
            hilo = 1;
          END
        else                      /* do high nible              */
          BEGIN
            *((Byte *) (move_ptr)) = ((chr * 16 ) BAND 0xf0)
                                     + *((Byte *) (move_ptr));
            hilo = 0;
            move_ptr--;
          END
      END

                                  /* adjust for scale dif.      */
    scale_dif = (pack_ptr -> pck_to_scale) -
                (pack_ptr -> pck_from1_scale);
    if (scale_dif NE 0)
      BEGIN
	status = rt_srpp(pack_ptr, scale_dif, 0); /*RS*/ /*1->0*/
    if (status NE RT_OK)
          BEGIN
            return(status);
          END
      END

    return(RT_OK);
  END

/**********************************************************************
*                                                                     *
*         FUNCTION NAME : Unpack a decimal number. (rtunpk)           *
*                                                                     *
*      INPUT PARAMETERS : pack_block                                  *
*                                                                     *
*     OUTPUT PARAMETERS : N/A                                         *
*                                                                     *
*          RESPONSE CODE: RT_OK                                       *
*                         RT_ERROR                                    *
*                                                                     *
*           DESCRIPTION : Unpack the decimal that is the "from1" parm *
*                         and put it in the "to" parm.                *
*                                                                     *
*      GLOBAL VARIABLES : N/A                                         *
*                                                                     *
*      FUNCTIONS CALLED : rt_srpp.                                    *
*                                                                     *
*                  DATE :                                             *
*                                                                     *
*              MODIFIED :                                             *
*                                                                     *
**********************************************************************/

Int rt_unpk (pack_ptr)

  Rt_pack_entry *pack_ptr;

  BEGIN

    Int sign;                     /* unpacked sign value        */
    Int hilo;                     /* switch for high/low nible  */
    Int move_size;                /* number of nibles to move   */
    Byte *move_ptr;               /* current work nible pointer */
    Byte *disp_ptr;               /* current work disp pointer  */
    Int chr;                      /* current character          */
    Byte *local_from;             /* from pointer for value     */

    Int scale_dif;                /* difference in scales       */
    Rt_pack_entry buffer_blk;     /* ot for srp operation       */
    Byte buffer I_ RT_SPCK_MAX _I;
                                  /* srp buffer                 */

    Int i;                        /* loop variable              */

    Int status;

    if (pack_ptr -> pck_from1_length > RT_SPCK_MAX OR
        pack_ptr -> pck_to_length > RT_SUNP_MAX)
      BEGIN                       /* wrong size field...        */
        return(RT_PACK_ERROR);
      END
                                  /* check for scaling          */
    local_from = pack_ptr -> pck_from1_ptr;
    scale_dif = (pack_ptr -> pck_to_scale) -
                (pack_ptr -> pck_from1_scale);
    if (scale_dif NE 0)
      BEGIN
        buffer_blk.pck_to_length = pack_ptr -> pck_from1_length;
        buffer_blk.pck_to_scale  = 0;
        buffer_blk.pck_to_ptr    = buffer;
        local_from = buffer;
        memcpy(buffer,
              (pack_ptr -> pck_from1_ptr),
              (pack_ptr -> pck_from1_length));
	status = rt_srpp(&buffer_blk, scale_dif, 0); /*RS*/ /*1->0*/
        if (status NE RT_OK)
          BEGIN
            return(status);
          END
      END

                                  /* handle the sign            */
    sign = RT_SUNP_USN;           /* should be RT_SUNP_POS ???  */
    move_ptr = local_from + (pack_ptr -> pck_from1_length) - 1;
    if ( ((*(Byte *) move_ptr) BAND 0x0f) == RT_SPCK_NEG)
      BEGIN
        sign = RT_SUNP_NEG;
      END
                                  /* clear the to field         */
    memset(pack_ptr -> pck_to_ptr,
           '0',
           pack_ptr -> pck_to_length);

                                  /* set the sign nible         */
    disp_ptr = (pack_ptr -> pck_to_ptr) +
               (pack_ptr -> pck_to_length) - 1;
    *((Byte *) disp_ptr) = sign;

                                  /* find num nibles to move    */
    move_size = ((pack_ptr -> pck_from1_length) * 2) - 1;
    if (move_size > (pack_ptr -> pck_to_length))
      BEGIN
        move_size = (pack_ptr -> pck_to_length);
      END

                                  /* set switch to hi byte      */
    hilo = 1;

                                  /* convert the rest           */
    for (i=0; i < move_size; i++)
      BEGIN
        if (hilo == 0)            /* do low nible               */
          BEGIN
            chr = *((Byte *) (move_ptr)) BAND 0x0f;
            hilo = 1;
          END
        else                      /* do high nible              */
          BEGIN
            chr = (*((Byte *) (move_ptr)) BAND 0xf0) / 16;
            hilo = 0;
            move_ptr--;
          END
        *((Byte *) disp_ptr) = (chr BOR *((Byte *) disp_ptr));
        disp_ptr--;
      END

    return(RT_OK);

  END

/**********************************************************************
*                                                                     *
*         FUNCTION NAME : Move packed to packed. (rtzapp)             *
*                                                                     *
*      INPUT PARAMETERS : pack_block                                  *
*                                                                     *
*     OUTPUT PARAMETERS : N/A                                         *
*                                                                     *
*          RESPONSE CODE: RT_OK                                       *
*                         RT_ERROR                                    *
*                                                                     *
*           DESCRIPTION : Copy the packed field in the "from1" parm   *
*                         to the packed field in the "to"  parm.      *
*                                                                     *
*      GLOBAL VARIABLES : N/A                                         *
*                                                                     *
*      FUNCTIONS CALLED : rt_unpck, rt_pack.                          *
*                                                                     *
*                  DATE :                                             *
*                                                                     *
*              MODIFIED :                                             *
*                                                                     *
**********************************************************************/

Int rt_zapp (pack_ptr)

  Rt_pack_entry *pack_ptr;

  BEGIN

    Rt_pack_entry buffer_blk;     /* conversion buffer          */

    Byte buffer I_ RT_SUNP_BUF _I;

    Int status;                   /* status return code         */

    buffer_blk.pck_from1_length = pack_ptr -> pck_from1_length;
    buffer_blk.pck_from1_scale  = pack_ptr -> pck_from1_scale;
    buffer_blk.pck_from1_ptr    = pack_ptr -> pck_from1_ptr;
    buffer_blk.pck_to_length = RT_SUNP_MAX;
    buffer_blk.pck_to_scale  = pack_ptr -> pck_to_scale;
    buffer_blk.pck_to_ptr    = buffer;

                                  /* convert decimal to display */
    status = rt_unpk(&buffer_blk);
    if (status NE RT_OK)
      BEGIN
        return(status);
      END

    buffer_blk.pck_from1_length = buffer_blk.pck_to_length;
    buffer_blk.pck_from1_scale  = buffer_blk.pck_to_scale;
    buffer_blk.pck_from1_ptr    = buffer_blk.pck_to_ptr;
    buffer_blk.pck_to_length = pack_ptr -> pck_to_length;
    buffer_blk.pck_to_scale  = pack_ptr -> pck_to_scale;
    buffer_blk.pck_to_ptr    = pack_ptr -> pck_to_ptr;

                                  /* convert display to packed  */

    status = rt_pack(&buffer_blk);
    if (status NE RT_OK)
      BEGIN
        return(status);
      END

    return(RT_OK);

  END

/**********************************************************************
*                                                                     *
*         FUNCTION NAME : Shift rounded. (rtsrpp)                     *
*                                                                     *
*      INPUT PARAMETERS : pack_block, shift_factor, round_factor.     *
*                                                                     *
*     OUTPUT PARAMETERS : N/A                                         *
*                                                                     *
*          RESPONSE CODE: RT_OK                                       *
*                         RT_ERROR                                    *
*                                                                     *
*           DESCRIPTION : Shift the packed field right or left and    *
*                         optionally round after right shift. The     *
*                         field to be shifted is the "to" field.      *
*                                                                     *
*      GLOBAL VARIABLES : N/A                                         *
*                                                                     *
*      FUNCTIONS CALLED : rt_cvap, rt_cvpa.                           *
*                                                                     *
*                  DATE :                                             *
*                                                                     *
*              MODIFIED :                                             *
*                                                                     *
**********************************************************************/

Int rt_srpp (pack_ptr, shift_factor, round_factor)

  Rt_pack_entry *pack_ptr;
  Int            shift_factor;
  Int            round_factor;

BEGIN

  Rt_pack_entry buffer_blk;       /* control block              */
  Byte          byte_array I_ BYTE_ARRAY_SIZE _I;
				  /* shift data array           */
  Int i, j;                       /* loop variables             */

  Int status;                     /* status return code         */

  Int remainder;                  /* remainder for rounding     */

                                  /* note the use of the to as  */
                                  /* both the "to" and "from"   */
                                  /* since this is a shift in-  */
                                  /* place.                     */
  buffer_blk.pck_from1_length = pack_ptr -> pck_to_length;
  buffer_blk.pck_from1_scale  = 0;
  buffer_blk.pck_from1_ptr    = pack_ptr -> pck_to_ptr;

  buffer_blk.pck_to_length    = sizeof(byte_array);
  buffer_blk.pck_to_scale     = 0;
  buffer_blk.pck_to_ptr       = (Byte *) byte_array;

				  /* convert packed to byte     */
				  /* array                      */
  status = rt_cvap(&buffer_blk);
  if (status NE RT_OK)
  BEGIN
    return(status);
  END

				  /* do the shift               */

  rt_shta(byte_array, shift_factor);

				  /* round not done       */

  if (shift_factor < 0 AND round_factor > 0)
  BEGIN
    return(RT_ERROR);
  END
				  /* convert byte array to      */
				  /* packed                     */

  buffer_blk.pck_from1_length = buffer_blk.pck_to_length;
  buffer_blk.pck_from1_scale  = buffer_blk.pck_to_scale;
  buffer_blk.pck_from1_ptr    = buffer_blk.pck_to_ptr;
  buffer_blk.pck_to_length    = pack_ptr -> pck_to_length;
  buffer_blk.pck_to_scale     = 0;     /* <=== important  */
  buffer_blk.pck_to_ptr       = pack_ptr -> pck_to_ptr;


  status = rt_cvpa(&buffer_blk);
  if (status NE RT_OK)
  BEGIN
    return(status);
  END

  return(RT_OK);

END


/**********************************************************************
*                                                                     *
*         FUNCTION NAME : Compare packed. (rtcpp)                     *
*                                                                     *
*      INPUT PARAMETERS : pack_block                                  *
*                                                                     *
*     OUTPUT PARAMETERS : N/A                                         *
*                                                                     *
*          RESPONSE CODE: RT_LT, RT_EQ, RT_GT                         *
*                         RT_ERROR                                    *
*                                                                     *
*           DESCRIPTION : Compare two numbers "from1" and "from2" as  *
*                         packed and return the results.              *
*                                                                     *
*      GLOBAL VARIABLES : N/A                                         *
*                                                                     *
*      FUNCTIONS CALLED : rt_zapp.                                    *
*                                                                     *
*                  DATE :                                             *
*                                                                     *
*              MODIFIED :                                             *
*                                                                     *
**********************************************************************/

Int rt_cpp (pack_ptr)

  Rt_pack_entry *pack_ptr;

  BEGIN
                                  /* comparison buffers          */
    Rt_pack_entry local_first_blk;
    Rt_pack_entry local_second_blk;
    Byte local_first_packed  I_ RT_SPCK_MAX _I;
    Byte local_second_packed I_ RT_SPCK_MAX _I;
    Byte local_first_sign;
    Byte local_second_sign;
    Int sign_offset;

    Int packed_scale;             /* scale for comparison       */

    Int status;                   /* status return code         */

                                  /* set compare scale          */
    packed_scale = pack_ptr -> pck_from1_scale;
    if ((pack_ptr -> pck_from2_scale) > packed_scale)
      BEGIN
        packed_scale = pack_ptr -> pck_from2_scale;
      END


    local_first_blk.pck_from1_length = pack_ptr -> pck_from1_length;
    local_first_blk.pck_from1_scale  = pack_ptr -> pck_from1_scale;
    local_first_blk.pck_from1_ptr    = pack_ptr -> pck_from1_ptr;
    local_first_blk.pck_to_length = sizeof(local_first_packed);
    local_first_blk.pck_to_scale  = packed_scale;
    local_first_blk.pck_to_ptr    = local_first_packed;

    local_second_blk.pck_from1_length = pack_ptr -> pck_from2_length;
    local_second_blk.pck_from1_scale  = pack_ptr -> pck_from2_scale;
    local_second_blk.pck_from1_ptr    = pack_ptr -> pck_from2_ptr;
    local_second_blk.pck_to_length = sizeof(local_second_packed);
    local_second_blk.pck_to_scale  = packed_scale;
    local_second_blk.pck_to_ptr    = local_second_packed;

    status = rt_zapp(&local_first_blk);
    if (status NE RT_OK)
      BEGIN
        return(status);
      END

    status = rt_zapp(&local_second_blk);
    if (status NE RT_OK)
      BEGIN
        return(status);
      END

                                  /* pick off the two signs     */
    sign_offset = sizeof(local_first_packed) - 1;
    local_first_sign = local_first_packed I_ sign_offset _I BAND
                       0x0f;
    local_second_sign = local_second_packed I_ sign_offset _I BAND
                       0x0f;
    if (local_first_sign == RT_SPCK_USN)
      BEGIN
        local_first_sign = RT_SPCK_POS;
      END
    if (local_second_sign == RT_SPCK_USN)
      BEGIN
        local_second_sign = RT_SPCK_POS;
      END

    status = RT_EQ;
                                  /* do the compares            */
    if (local_first_sign == local_second_sign)
      BEGIN
        if (local_first_sign == RT_SPCK_POS)
          BEGIN
            status = memcmp(local_first_packed,
                            local_second_packed,
                            sizeof(local_first_packed));
          END
        else
          BEGIN
            status = memcmp(local_second_packed,
                            local_first_packed,
                            sizeof(local_first_packed));
          END
      END
    else
      BEGIN
        if (local_first_sign == RT_SPCK_POS)
          BEGIN
            status = RT_GT;
          END
        else
          BEGIN
            status = RT_LT;
          END
      END
                                  /* correct status to be sure  */
                                  /* that only RT_LT, RT_EQ, or */
                                  /* or RT_GT are returned.     */
    if (status >= RT_GT)
    BEGIN
      status = RT_DEC_GT;
    END
    else if (status <= RT_LT)
    BEGIN
      status = RT_DEC_LT;
    END
    else if (status == RT_EQ)
    BEGIN
      status = RT_DEC_EQ;
    END

    return(status);

  END


/**********************************************************************
*                                                                     *
*         FUNCTION NAME : Packed math: rt_mthp.                       *
*                                                                     *
*      INPUT PARAMETERS : pack_block and operation.,                  *
*                                                                     *
*     OUTPUT PARAMETERS : none.                                       *
*                                                                     *
*          RESPONSE CODE:                                             *
*                         0: ok.                                      *
*                                                                     *
*           DESCRIPTION : "to" = "from1" op "from2" (see pack_blk).   *
*                                                                     *
*      GLOBAL VARIABLES :                                             *
*                                                                     *
*      FUNCTIONS CALLED : rt_cvbp, rt_cvdp.                           *
*                                                                     *
*                  DATE :                                             *
*                                                                     *
*              MODIFIED :                                             *
*                                                                     *
**********************************************************************/

Int rt_mthp( pack_ptr, operation)

  Rt_pack_entry *pack_ptr;
  Char          operation;

  BEGIN
                                  /* these two sets of OTs are  */
                                  /* used to get the correct    */
                                  /* conversions to symulate    */
				  /* packed math.               */

    Rt_pack_entry local_1_blk, local_2_blk, local_to_blk;

    Signed_Char a_field I_ BYTE_ARRAY_SIZE _I; /* the A byte array */
    Signed_Char b_field I_ BYTE_ARRAY_SIZE _I; /* the B byte array */
    Signed_Char c_field I_ BYTE_ARRAY_SIZE _I; /* the C byte array */

    Int local_1_shift;            /* amount to shift from_1     */
    Int local_2_shift;            /* amount to shift from_2     */
    Int local_to_shift;           /* amount to shift "to"       */
    Int max_dec_places;           /* max of any field in calc   */

    Int status;                   /* status return              */
    Int dec_places;               /* dec places with divide     */
    Int spec_shift;               /* shift to keep dec places   */

    Int i, j;                     /* loop variables             */
    Long carry;                   /* carry in math              */

                                  /* these packentries are used */
                                  /* to deal with the input     */
                                  /* fields with the scale = 0  */

    local_1_blk.pck_from1_ptr    = pack_ptr -> pck_from1_ptr;
    local_1_blk.pck_from1_length = pack_ptr -> pck_from1_length;
    local_1_blk.pck_from1_scale  = 0;
    local_1_blk.pck_to_ptr    = a_field;
    local_1_blk.pck_to_length = sizeof(a_field);
    local_1_blk.pck_to_scale  = 0;

    local_2_blk.pck_from1_ptr    = pack_ptr -> pck_from2_ptr;
    local_2_blk.pck_from1_length = pack_ptr -> pck_from2_length;
    local_2_blk.pck_from1_scale  = 0;
    local_2_blk.pck_to_ptr    = b_field;
    local_2_blk.pck_to_length = sizeof(b_field);
    local_2_blk.pck_to_scale  = 0;

                                  /* convert the input fields   */
    status = rt_cvap(&local_1_blk);
    if (status NE RT_OK)
      return(status);

    status = rt_cvap(&local_2_blk);
    if (status NE RT_OK)
      return(status);
				  /* do "before" calc shifts    */
    switch (operation)
      BEGIN
        case ('+'):
	case ('-'):
		    max_dec_places = pack_ptr -> pck_to_scale;
		    if (pack_ptr -> pck_from1_scale >
			max_dec_places)
		    BEGIN
		      max_dec_places = pack_ptr -> pck_from1_scale;
		    END
		    if (pack_ptr -> pck_from2_scale >
			max_dec_places)
		    BEGIN
		      max_dec_places = pack_ptr -> pck_from2_scale;
		    END
		    local_1_shift = max_dec_places -
                                    (pack_ptr -> pck_from1_scale);
		    local_2_shift = max_dec_places -
				    (pack_ptr -> pck_from2_scale);
		    local_to_shift = (pack_ptr -> pck_to_scale) -
				     max_dec_places;
				  /* shift byte array as needed */
		    rt_shta(a_field, local_1_shift);
		    rt_shta(b_field, local_2_shift);
                    break;
        case ('*'):
                    local_to_shift = (pack_ptr -> pck_to_scale) -
                                     ((pack_ptr -> pck_from1_scale) +
                                      (pack_ptr -> pck_from2_scale));
                    break;
	case ('/'):
				  /* "dividend"                 */
				  /* ---------- = "quotient"    */
				  /*  "divisor"                 */

				  /* we want the divisor to     */
				  /* have atleast the scale as  */
				  /* the quotient + 1           */

				  /* need to make divisor not  */
				  /* be less than 1, so, scale */
				  /* the dividend and divisor  */
				  /* by needed number          */
		    if (pack_ptr -> pck_to_scale >
			pack_ptr -> pck_from2_scale)
		    BEGIN
		      spec_shift = pack_ptr -> pck_to_scale;
		    END
		    else
		    BEGIN
		      spec_shift = pack_ptr -> pck_from2_scale;
		    END
		    rt_shta(b_field, spec_shift);
		    rt_shta(a_field, spec_shift);

				  /* compute local_shift which  */
				  /* is the shift to be done    */
				  /* after the division.        */

		    local_to_shift = (pack_ptr -> pck_to_scale) -
				     ((pack_ptr -> pck_from1_scale) -
				      (pack_ptr -> pck_from2_scale));
                    break;
      END
				  /* at this point we can do    */
				  /* math like non-scaled int   */
				  /* numbers.  they are stored  */
				  /* as char arrays.            */


    switch (operation)
      BEGIN
	case ('+'):                /* Packed ADDITION           */
	  status = rt_adda(c_field, a_field, b_field);
	  if (status NE RT_OK)
	  BEGIN
	    return(status);
	  END

	  break;

	case ('-'):                /* Packed SUBTRACTION        */
	  status = rt_suba(c_field, a_field, b_field);
	  if (status NE RT_OK)
	  BEGIN
	    return(status);
	  END

	  break;

	case ('*'):               /* Packed MULTIPLICATION      */

	  status = rt_mula(c_field, a_field, b_field);
	  if (status NE RT_OK)
	  BEGIN
	    return(status);
	  END

	  break;

	case ('/'):               /* Packed DIVISION            */

	  status = rt_diva(c_field, a_field, b_field,
			   &dec_places);
	  if (status NE RT_OK)
	  BEGIN
	    return(status);
	  END

	  break;
      END
                                  /* do after calc shifts       */
    switch (operation)
      BEGIN
	case ('+'):
	case ('-'):
        case ('*'):
				  /* shift byte array as needed */
		    rt_shta(c_field, local_to_shift);
		    break;
	case ('/'):
				  /* local_to_shift gives the   */
				  /* shift to correct the result*/
				  /* to the desried number of   */
				  /* decimal places. BUT, we    */
				  /* must adjust this because   */
				  /* the divide will return a   */
				  /* variable number of dec.    */
				  /* places in the result!      */

		    local_to_shift = local_to_shift -
				     dec_places;
		    rt_shta(c_field, local_to_shift);
		    break;
      END

    local_to_blk.pck_from1_ptr    = c_field;
    local_to_blk.pck_from1_length = sizeof(c_field);
    local_to_blk.pck_from1_scale  = 0;
    local_to_blk.pck_to_ptr    = pack_ptr -> pck_to_ptr;
    local_to_blk.pck_to_length = pack_ptr -> pck_to_length;
    local_to_blk.pck_to_scale  = 0;

                                  /* convert the output field   */
    status = rt_cvpa(&local_to_blk);
    if (status NE RT_OK)
      return(status);

    return(RT_OK);

  END


/**********************************************************************
*                                                                     *
*         FUNCTION NAME : Convert packed to array. (rtcvap)           *
*                                                                     *
*      INPUT PARAMETERS : pack_block                                  *
*                                                                     *
*     OUTPUT PARAMETERS : N/A                                         *
*                                                                     *
*          RESPONSE CODE: RT_OK                                       *
*                         RT_ERROR                                    *
*                                                                     *
*           DESCRIPTION : Convert the decimal in the "from1" parm     *
*                         and put it in the "to" parm as a Byte       *
*                         array, one nibble per Byte                  *
*                                                                     *
*      GLOBAL VARIABLES : N/A                                         *
*                                                                     *
*      FUNCTIONS CALLED :                                             *
*                                                                     *
*                  DATE :                                             *
*                                                                     *
*              MODIFIED :                                             *
*                                                                     *
**********************************************************************/

Int rt_cvap (pack_ptr)

  Rt_pack_entry *pack_ptr;

BEGIN

    /* the input packed number is converted to a simple Byte    */
    /* array where each nible (packed digit) becomes an integer */
    /* number.  The sign nible is stored as 1 for pos and -1    */
    /* for neg.                                                 */
    /*                                                          */
    /* An example:                                              */
    /*                                                          */
    /* The backed number 27, stored as:                         */
    /*                x'0000027c'                               */
    /*  would become:                                           */
    /*  int Byte  I_ size _I = { 0, 0, 0,  .. 0, 2, 7, 1};      */
    /*                                                          */

  Int size;                       /* digits in packed number    */
  Byte *byte_ptr;                 /* pointer to byte in packed  */
				  /* number                     */
  Int hilo;                       /* swith for high/low nible   */
  Signed_Char *array_ptr;         /* pointer to array element   */

  Int skip_ints;                  /* number of integers to set  */
				  /* to zero since packed field */
				  /* may be small               */

  Int i;                          /* loop variable              */

				  /* find size and location of  */
				  /* the packed number          */
  size = (pack_ptr -> pck_from1_length * 2) - 1;
  byte_ptr = (Byte *) pack_ptr -> pck_from1_ptr;

				  /* find the Int array, we know*/
				  /* it is BYTE_ARRAY_SIZE bytes*/
  array_ptr = (Byte *) pack_ptr -> pck_to_ptr;

				   /* let's convert!            */
				   /* step1- zero extra integers*/
				   /* in the array              */
  skip_ints = (BYTE_ARRAY_SIZE - 1) - size;
  for (i=0; i < skip_ints; i++)
  BEGIN
    *(array_ptr + i) = 0;
  END

  hilo = 1;                       /* start with hi nible        */
  for (i=skip_ints; i < (BYTE_ARRAY_SIZE -1); i++)
  BEGIN
    if (hilo == 0)
    BEGIN                         /* get low order nible        */
      *(array_ptr + i) = *((Byte *) byte_ptr) BAND 0x0f;
      hilo = 1;
      byte_ptr++;
    END
    else
    BEGIN                         /* get high order nible       */
      *(array_ptr + i) = (*((Byte *) byte_ptr) BAND 0xf0) >> 4;
      hilo = 0;
    END
  END
				  /* process the sign           */
  if ((* ((Byte *) byte_ptr) BAND 0x0f) == 0x0d)
  BEGIN
    *(array_ptr + (BYTE_ARRAY_SIZE - 1)) = RT_NEG;
  END
  else
  BEGIN
    *(array_ptr + (BYTE_ARRAY_SIZE - 1)) = RT_POS;
  END

  return(RT_OK);

END


/**********************************************************************
*                                                                     *
*         FUNCTION NAME : Convert array to packed. (rtcvpa)           *
*                                                                     *
*      INPUT PARAMETERS : pack_block                                  *
*                                                                     *
*     OUTPUT PARAMETERS : N/A                                         *
*                                                                     *
*          RESPONSE CODE: RT_OK                                       *
*                         RT_ERROR                                    *
*                                                                     *
*           DESCRIPTION : Convert the Byte array in the "from1"       *
*                         parm to a packed number in "to" parm.       *
*                         (see comments in rt_cvap header!)           *
*                                                                     *
*      GLOBAL VARIABLES : N/A                                         *
*                                                                     *
*      FUNCTIONS CALLED :                                             *
*                                                                     *
*                  DATE :                                             *
*                                                                     *
*              MODIFIED :                                             *
*                                                                     *
**********************************************************************/

Int rt_cvpa (pack_ptr)

  Rt_pack_entry *pack_ptr;

BEGIN

    /* see comment in rt_cvap, this function converts the       */
    /* opposite way.                                            */

  Int size;                       /* digits in packed number    */
  Byte *byte_ptr;                 /* pointer to byte in packed  */
				  /* number                     */
  Int hilo;                       /* swith for high/low nible   */
  Signed_Char *array_ptr;         /* pointer to array element   */
  Int skip_ints;                  /* number of integers to set  */
				  /* to zero since packed field */
				  /* may be smaller             */
  Int sign;                       /* sign indicator             */
  Int i;                          /* loop variable              */

				  /* find size and location of  */
				  /* the packed number          */
  size = (pack_ptr -> pck_to_length * 2) - 1;
  byte_ptr = (Byte *) pack_ptr -> pck_to_ptr;

				  /* find the Int array, we know*/
				  /* it is 16 Ints in size      */
  array_ptr = (Byte *) pack_ptr -> pck_from1_ptr;

				   /* let's convert!            */
				   /* step1- zero the packed    */
				   /* field                     */
  memset(pack_ptr -> pck_to_ptr, 0x00, pack_ptr -> pck_to_length);

				   /* number of integers to     */
				   /* skip                      */
  skip_ints = (BYTE_ARRAY_SIZE - 1) - size;

  hilo = 1;                       /* start with hi nible        */
  for (i=0;i < size; i++)
  BEGIN
    if (hilo == 0)
    BEGIN                         /* set low order nible        */
      *((Byte *) byte_ptr) = *((Byte *) byte_ptr) +
			     *(array_ptr + i + skip_ints);
      hilo = 1;
      byte_ptr++;
    END
    else
    BEGIN                         /* get high order nible       */
      *((Byte *) byte_ptr) = *(array_ptr + i + skip_ints) << 4;
      hilo = 0;
    END
  END
				  /* process the sign           */
  if (*(array_ptr + (BYTE_ARRAY_SIZE - 1)) == RT_POS)
  BEGIN
    sign = RT_SPCK_POS;
  END
  else
  BEGIN
    sign = RT_SPCK_NEG;
  END

  *((Byte *) byte_ptr) = *((Byte *) byte_ptr) + sign;

  return(RT_OK);

END


/**********************************************************************
*                                                                     *
*         FUNCTION NAME : Shift pack array. (rt_shta)                 *
*                                                                     *
*      INPUT PARAMETERS : packed array pointer                        *
*                         shift value                                 *
*                                                                     *
*     OUTPUT PARAMETERS : N/A                                         *
*                                                                     *
*          RESPONSE CODE: RT_OK                                       *
*                         RT_ERROR                                    *
*                                                                     *
*           DESCRIPTION : Shift packed array right or left.           *
*                         if pos, then right. if neg then left        *
*                                                                     *
*      GLOBAL VARIABLES : N/A                                         *
*                                                                     *
*      FUNCTIONS CALLED :                                             *
*                                                                     *
*                  DATE :                                             *
*                                                                     *
*              MODIFIED :                                             *
*                                                                     *
**********************************************************************/

Int rt_shta (byte_array, shift_factor)

  Signed_Char byte_array I_ _I;
  Int shift_factor;

BEGIN

  Int i, j, not_zero_flag;

  if (shift_factor > 0)
  BEGIN
    for (i=0; i < shift_factor; i++)
    BEGIN                         /* shift left                 */
      /* num = num * 10; */
      for (j=0; j < (BYTE_ARRAY_SIZE - 2); j++)
      BEGIN
	byte_array I_ j _I = byte_array I_ j + 1 _I;
      END
				  /* fill in low order digit    */
      byte_array I_ (BYTE_ARRAY_SIZE - 2) _I = 0;
    END
  END
  else
  BEGIN
    not_zero_flag = 1;
    for (i=0; i < (-shift_factor); i++)
    BEGIN                       /* shift right                */
      if (i == 0)
      BEGIN
	not_zero_flag = 0;
      END
      /* remainder = byte_array I_ BYTE_ARRAY_SIZE - 2 _I; */
      /* num = num /10; */
      for (j=(BYTE_ARRAY_SIZE - 2); j >= 1; j--)
      BEGIN
	byte_array I_ j _I = byte_array I_ j - 1 _I;
	if (byte_array I_ j _I NE 0)
	BEGIN
	  not_zero_flag = 1;
	END
      END
    END
    byte_array I_ 0 _I = 0;
    if (not_zero_flag == 0)
    BEGIN                         /* there is nothing in the    */
				  /* world that smells worse    */
				  /* than a negative zero!      */
      byte_array I_ BYTE_ARRAY_SIZE - 1 _I = RT_POS;
    END
  END

  return(RT_OK);

END


/**********************************************************************
*                                                                     *
*         FUNCTION NAME : Add packed array. (rt_adda)                 *
*                                                                     *
*      INPUT PARAMETERS : 3 packed arrays                             *
*                                                                     *
*     OUTPUT PARAMETERS : N/A                                         *
*                                                                     *
*          RESPONSE CODE: RT_OK                                       *
*                         RT_ERROR                                    *
*                                                                     *
*           DESCRIPTION : array c = array a + array b.                *
*                                                                     *
*      GLOBAL VARIABLES : N/A                                         *
*                                                                     *
*      FUNCTIONS CALLED :                                             *
*                                                                     *
*                  DATE :                                             *
*                                                                     *
*              MODIFIED :                                             *
*                                                                     *
**********************************************************************/

Int rt_adda (c_field, a_field, b_field)

  Signed_Char a_field I_ _I;
  Signed_Char b_field I_ _I;
  Signed_Char c_field I_ _I;

BEGIN

  Int i, j;                     /* loop variables             */
  Long carry;                   /* carry in math              */
  Int loop_digits;              /* 0 .. loop_digits           */
  Int sign_offset;              /* offset to sign             */

  loop_digits = BYTE_ARRAY_SIZE - 2;
  sign_offset = BYTE_ARRAY_SIZE - 1;

  carry = 0;

  for (i=loop_digits; i >=0; i--)
  BEGIN
    if (a_field I_ sign_offset _I == RT_NEG)
    BEGIN       /* cascade the sign in a      */
      a_field I_ i _I = - a_field I_ i _I;
    END
    if (b_field I_ sign_offset _I == RT_NEG)
    BEGIN       /* cascade the sign in b      */
      b_field I_ i _I = - b_field I_ i _I;
    END
    c_field I_ i _I = carry +
		      a_field I_ i _I +
		      b_field I_ i _I;
    carry = 0;
    if (c_field I_ i _I > 9)
    BEGIN
      c_field I_ i _I = c_field I_ i _I - 10;
      carry = 1;
    END
    else if (c_field I_ i _I < 0)
    BEGIN
      c_field I_ i _I = c_field I_ i _I + 10;
      carry = -1;
    END
  END
  if (carry == -1)
  BEGIN                           /* we have a neg number that  */
				  /* needs to be expressed in   */
				  /* the correct format! So,    */
				  /* convert from 2s-comp to a  */
				  /* real neg number            */
    carry = 1;  /* really add 1 */
    for (i=loop_digits; i >= 0; i--)
    BEGIN
      c_field I_ i _I = 9 - c_field I_ i _I +
					  carry;
      carry = 0;
      if (c_field I_ i _I > 9)
      BEGIN
	c_field I_ i _I -= 10;
	carry = 1;
      END
    END
    c_field I_ sign_offset _I = RT_NEG;
  END
  else
  BEGIN
    c_field I_ sign_offset _I = RT_POS;
  END

  return(RT_OK);

END


/**********************************************************************
*                                                                     *
*         FUNCTION NAME : Sub packed array. (rt_suba)                 *
*                                                                     *
*      INPUT PARAMETERS : 3 packed arrays                             *
*                                                                     *
*     OUTPUT PARAMETERS : N/A                                         *
*                                                                     *
*          RESPONSE CODE: RT_OK                                       *
*                         RT_ERROR                                    *
*                                                                     *
*           DESCRIPTION : array c = array a - array b.                *
*                                                                     *
*      GLOBAL VARIABLES : N/A                                         *
*                                                                     *
*      FUNCTIONS CALLED :                                             *
*                                                                     *
*                  DATE :                                             *
*                                                                     *
*              MODIFIED :                                             *
*                                                                     *
**********************************************************************/

Int rt_suba (c_field, a_field, b_field)

  Signed_Char a_field I_ _I;
  Signed_Char b_field I_ _I;
  Signed_Char c_field I_ _I;

BEGIN
				  /* working field for subtract */
  Signed_Char newb_field I_ BYTE_ARRAY_SIZE _I;

  memcpy(newb_field, b_field, sizeof(newb_field));

				   /* change to addition!       */
  newb_field I_ BYTE_ARRAY_SIZE - 1 _I =
	     - newb_field I_ BYTE_ARRAY_SIZE - 1 _I;

  return(rt_adda(c_field, a_field, newb_field));

END


/**********************************************************************
*                                                                     *
*         FUNCTION NAME : Mul packed array. (rt_mula)                 *
*                                                                     *
*      INPUT PARAMETERS : 3 packed arrays                             *
*                                                                     *
*     OUTPUT PARAMETERS : N/A                                         *
*                                                                     *
*          RESPONSE CODE: RT_OK                                       *
*                         RT_ERROR                                    *
*                                                                     *
*           DESCRIPTION : array c = array a * array b.                *
*                                                                     *
*      GLOBAL VARIABLES : N/A                                         *
*                                                                     *
*      FUNCTIONS CALLED :                                             *
*                                                                     *
*                  DATE :                                             *
*                                                                     *
*              MODIFIED :                                             *
*                                                                     *
**********************************************************************/

Int rt_mula (c_field, a_field, b_field)

  Signed_Char a_field I_ _I;
  Signed_Char b_field I_ _I;
  Signed_Char c_field I_ _I;

BEGIN

  Int i, j, k, jj;                /* loop variables             */
  Int big_bertha;                 /* optimization switch        */
  Long carry;                     /* carry in math              */

  Long tm I_ (BYTE_ARRAY_SIZE / 4) _I;  /* multiply top field   */
  Long bm I_ (BYTE_ARRAY_SIZE / 4) _I;  /* multiply bottom fld. */
  Long part_result I_ (BYTE_ARRAY_SIZE / 4) _I
		   I_ (BYTE_ARRAY_SIZE / 4) _I;
  Long pr;

				  /* multiplication partials    */
				  /* must be twice size of other*/
				  /* areas                      */
  Long sum I_ (BYTE_ARRAY_SIZE / 2) _I;

  Int loop_digits;                /* 00 .. loop_digits          */
  Int sign_offset;                /* offset to sign             */

  loop_digits = BYTE_ARRAY_SIZE - 2;
  sign_offset = BYTE_ARRAY_SIZE - 1;

	/* simulate packed mult from byte array by loading the  */
	/* real digits in each array into long fields that are  */
	/* them multiplied by the other long fields to get      */
	/* results.  these results are then put in the c_field  */
	/* byte array                                           */

	/* basic idea is:                                       */
	/*                            tm1   tm2   tm3   tm4     */
	/*                          x bm1   bm2   bm3   bm4     */
	/*                            ---------------------     */
	/*                  tm1*bm4 tm2*bm4 tm3*bm4 tm4*bm4     */
	/*          tm1*bm3 tm2*bm3 tm3*bm3 tm4*bm3             */
	/*  tm1*bm2 tm2*bm2 tm3*bm3 tm4*bm4                     */
	/*     etc.                                             */
	/*  --------------------------------------------------  */
	/*   summing these columns gives the answer!!!!!        */
	/*   (just like it is done by hand, except in units of  */
	/*   10000 instead of 10!)                              */

				  /* note, for this mult algo.  */
				  /* to work, a LONG on this    */
				  /* platform MUST hold at least*/
				  /* 8 digits without any trunc */
				  /* ation.                     */

				  /* first, convert the numbers */
				  /* from byte arrays to real   */
				  /* integers in the tm and bm  */
				  /* arrays.  3 digits in first */
				  /* integer, 4 digits in the   */
				  /* next 3 integers            */

  for (i=0; i < (BYTE_ARRAY_SIZE / 4); i++)
  BEGIN
    tm I_ i _I = 0;
    bm I_ i _I = 0;
  END

  for (i=0; i <= loop_digits; i++)
  BEGIN
				  /* put on 3 digits in the     */
				  /* first tm/bm slot           */
   j = (i + 1) / 4;
   tm I_ j _I *= 10;
   tm I_ j _I += a_field I_ i _I;
   bm I_ j _I *= 10;
   bm I_ j _I += b_field I_ i _I;
  END

				  /* here is the place to check */
				  /* for simple-er multiply     */
				  /* case                       */
  big_bertha = 0;
  for (i=0; i < ((BYTE_ARRAY_SIZE / 4) - 1); i++)
  BEGIN
    if (tm I_ i _I NE 0 OR
	bm I_ i _I NE 0)
    BEGIN
      big_bertha = 1;
      break;                      /* rats! big numbers must use */
				  /* "big bertha" multiply      */
    END
  END

  if (big_bertha EQ 0)
  BEGIN                           /* only tm(last) and bm(last) */
				  /* have values, so just do it */
    /* printf("* opt done\n");  */
    for (i=0; i < (BYTE_ARRAY_SIZE / 2); i++)
    BEGIN
      sum I_ i _I = 0;
    END
    j = (BYTE_ARRAY_SIZE / 4) - 1;
    jj = (BYTE_ARRAY_SIZE / 2) - 1;
    sum I_ jj _I = tm I_ j _I * bm I_ j _I;
    sum I_ jj - 1 _I = sum I_ jj _I / 10000;
    sum I_ jj _I = sum I_ jj _I % 10000;

  END
  else
  BEGIN

				  /* second, matrix tm * bm     */
    for (i=0; i < (BYTE_ARRAY_SIZE / 4) ;i++)
    BEGIN
      for (j=0; j < (BYTE_ARRAY_SIZE / 4); j++)
      BEGIN
	part_result I_ j  _I I_  i  _I =
		    tm I_ j _I * bm I_ i _I;
      END
    END

				  /* third, sum up the cells we */
				  /* care about. See comments   */
				  /* below....                  */

  /* this is a bit ughly; but, it is basically doing this */
  /*                                                      */
  /*              pr(1,1) pr(1,2) pr(1,3) pr(1,4) ...     */
  /*              pr(2,1) pr(2,2) pr(2,3) pr(2,4) ...     */
  /*              pr(3,1) pr(3,2) pr(3,3) pr(3,4) ...     */
  /*              pr(4,1) pr(4,2) pr(4,3) pr(4,4) ...     */
  /*              .       .       .       .       .       */
  /*              .       .       .       .       .       */
  /*                  where pr = part_result              */
  /*                                                      */
  /*  adding up the diagonals:                            */
  /*                                                      */
  /*           sum (last) = pr(last,last)                 */
  /*           sum (last-1) = pr(last,last-1) +           */
  /*                          pr(last-1,last)             */
  /*  and so forth.....                                   */
  /*                                                      */

				  /* Note, must check for carr- */
				  /* ing in these sums.  each   */
				  /* sum is at max 4 digits     */

				  /* sum part 1:                */
				  /*  + = numbers summed        */
				  /* (1,1) . . . + (1,4)        */
				  /*       . . + +              */
				  /*       . + + +              */
				  /*       + + + + (4,4)        */
				  /*     S S S S                */
    carry = 0;
    for (j = ((BYTE_ARRAY_SIZE / 4) -1); j >= 0; j--)
    BEGIN
				  /* jj offsets to second half  */
				  /* of the sum vector          */
      jj = j + (BYTE_ARRAY_SIZE / 4);
      sum I_ jj _I = carry;
      k = (BYTE_ARRAY_SIZE / 4) - 1;
      for (i = j; i < (BYTE_ARRAY_SIZE / 4); i++)
      BEGIN
	pr = part_result I_ i _I I_ k _I;
	sum I_ jj _I = sum I_ jj _I + pr;
	k--;
      END
      carry = sum I_ jj _I / 10000;
      sum I_ jj _I = sum I_ jj _I % 10000;
    END

				  /* sum part 2:                */
				  /*  + = numbers summed        */
				  /* (1,1) + + + . (1,4)        */
				  /*     S + + . .              */
				  /*     S + . . .              */
				  /*     S . . . . (4,4)        */
				  /*                            */

				  /* don't do the main diagonal */
				  /* twice!                     */
    /* this goes to 64 digits -- we don't need it done
    for (j = ((BYTE_ARRAY_SIZE / 4) - 2); j >= 0; j--)
    BEGIN
      jj = j + 1;
      sum I_ jj _I = carry;
      k = 0;
      for (i = j; i >= 0; i--)
      BEGIN
	pr = part_result I_ k _I I_ i _I;
	sum I_ jj _I = sum I_ jj _I + pr;
	k++;
      END
      carry = sum I_ jj _I / 10000;
      sum I_ jj _I = sum I_ jj _I % 10000;
    END
    sum I_ 0 _I = carry;
   ..... so let's not do it */

  END /* of big bertha if */

				  /* fourth, put the intgers in */
				  /* sum array into the byte    */
				  /* array.                     */

  for (i=loop_digits; i >=0; i--)
  BEGIN
    j = (i + 1) / 4;
    jj = j + (BYTE_ARRAY_SIZE / 4);
				  /* get low order digit        */
    c_field I_ i _I = sum I_ jj _I % 10;
				  /* now toss it....            */
    sum I_ jj _I = sum I_ jj _I / 10;
  END

				  /* fifth, process the sign    */
  c_field I_ sign_offset _I = a_field I_ sign_offset _I *
			      b_field I_ sign_offset _I;

  return(RT_OK);

END


/**********************************************************************
*                                                                     *
*         FUNCTION NAME : Div packed array. (rt_diva)                 *
*                                                                     *
*      INPUT PARAMETERS : 3 packed arrays                             *
*                                                                     *
*     OUTPUT PARAMETERS : N/A                                         *
*                                                                     *
*          RESPONSE CODE: RT_OK                                       *
*                         RT_ERROR                                    *
*                                                                     *
*           DESCRIPTION : array c = array a / array b.                *
*                                                                     *
*      GLOBAL VARIABLES : N/A                                         *
*                                                                     *
*      FUNCTIONS CALLED :                                             *
*                                                                     *
*                  DATE :                                             *
*                                                                     *
*              MODIFIED :                                             *
*                                                                     *
**********************************************************************/

Int rt_diva (c_field, a_field, b_field, dec_places_ptr)

  Signed_Char a_field I_ _I;
  Signed_Char b_field I_ _I;
  Signed_Char c_field I_ _I;
  Int    *dec_places_ptr;

BEGIN
	  /* this division algorythm was taken from Knuth's     */
	  /* book: "Seminumerical Algorythms", in the chapter   */
	  /* on Arithmetic, section 4.3.1, "The Classical       */
	  /* Algorythms".  This is roughly algorythm D.         */

	  /* note that the a_field and b_field are ready to be  */
	  /* processed and the answer goes in c_field.  this    */
	  /* algo is for un-signed numbers, so the sign is      */
	  /* handled after the Knuth algo is done!              */

	  /* u is the a_field and v is the b_field              */
	  /* the field n contains the number of decimal places  */
	  /* in the c_field which is also the number of         */
	  /* significant digits in the b_field!                 */

  Int i, j, status;               /* working variables          */

  Int sign_offset, loop_digits, max_digits, r_shift;

				  /* division work fields       */
  Long b, d, m, n, n_offset, qq;
  Long u_1, u_2, u_12, u_3, v_1, v_2;

				  /* copies of input            */
  Signed_Char u_field I_ BYTE_ARRAY_SIZE _I;
  Signed_Char v_field I_ BYTE_ARRAY_SIZE _I;

				  /* scratch byte arrays        */
  Signed_Char d_field I_ BYTE_ARRAY_SIZE _I;
  Signed_Char qq_field I_ BYTE_ARRAY_SIZE _I;
  Signed_Char r_field I_ BYTE_ARRAY_SIZE _I;
  Signed_Char s_field I_ BYTE_ARRAY_SIZE _I;

  loop_digits = BYTE_ARRAY_SIZE - 2;
  max_digits  = BYTE_ARRAY_SIZE - 1;
  sign_offset = BYTE_ARRAY_SIZE - 1;
				  /* make changable working     */
				  /* copies of the input arrays */
  memcpy(u_field, a_field, sizeof(u_field));
  memcpy(v_field, b_field, sizeof(v_field));
  u_field I_ sign_offset _I = 1;  /* make both positive!        */
  v_field I_ sign_offset _I = 1;

  b = 10;                          /* radix of 10               */
  n = max_digits;                  /* max number of significant */
				   /* digits in v_field         */
				   /* now find real number of   */
				   /* significant digits        */
  for (i=0; i <= loop_digits; i++)
  BEGIN
    if (v_field I_ i _I NE 0)
    BEGIN
      break;
    END
    n--;
  END

  if (n == 0)
  BEGIN                           /* if n is 0, then v_field is */
				  /* also 0                     */
    return(RT_DIVIDE_ZERO);
  END
  if (n == max_digits)
  BEGIN                           /* this algo will fail to do  */
				  /* division!                  */
    return(RT_ERROR);
  END

  m = max_digits - n;             /* now can set m              */
  n_offset = m;                   /* number of digits in v_field*/
				  /* to skip                    */

  /* STEP D1, normalize */
				  /* compute normalization      */
				  /* constant                   */
  d = b / (v_field I_ n_offset _I + 1);
#ifdef TRACE_DIV
  printf(" n: %ld, d: %ld\n", n, d);
#endif

				  /* convert d to byte array    */
				  /* and clear qq_field for     */
				  /* later use.                 */
  for (i=0; i < loop_digits; i++)
  BEGIN
    d_field I_ i _I = 0;
    qq_field I_ i _I = 0;
  END
  d_field I_ BYTE_ARRAY_SIZE -2 _I = d;
  d_field I_ sign_offset _I = 1;   /* pos. number */

				  /* u_field * d_field          */
  status = rt_mula(u_field, u_field, d_field);
  if (status NE RT_OK)
  BEGIN
    return(status);
  END
				  /* v_field * d_field          */
  status = rt_mula(v_field, v_field, d_field);
  if (status NE RT_OK)
  BEGIN
    return(status);
  END

#ifdef TRACE_DIV
   printf(" u_field * dd: "); SHOW_BA(u_field);
   printf(" v_field * dd: "); SHOW_BA(v_field);
#endif
				  /* set c field as positive    */
  c_field I_ sign_offset _I = 1;

  /* STEP D2, Intialize j (really start loop) */

  for (j = 0; j <= loop_digits ; j++)
  BEGIN

  /* STEP D3, Calculate qq */
    u_1 = u_field I_ j _I;
    if ((j + 1) < max_digits)
    BEGIN
      u_2 = u_field I_ j + 1 _I;
    END
    else
    BEGIN
      u_2 = 0;
    END
    if ((j + 2) < max_digits)
    BEGIN
      u_3 = u_field I_ j + 2 _I;
    END
    else
    BEGIN
      u_3 = 0;
    END
    u_12 = u_1 * b + u_2;
    v_1  = v_field I_ n_offset _I;
    if (n > 1)
    BEGIN
      v_2  = v_field I_ n_offset + 1 _I;
    END
    else
    BEGIN
      v_2 = 0;
    END

    if (u_1 == v_1)
    BEGIN
      qq = b - 1;
    END
    else                          /* need these variables so    */
				  /* that we can "look" past the*/
				  /* end of the actual arrays   */
    BEGIN
      qq = u_12 / v_1;
    END
#ifdef TRACE_DIV
  printf(" j is: %d\n", j);
  printf(" trial qq is: %ld\n", qq);
#endif
				  /* "tune" qq                  */
    while ( (v_2 * qq) > (b*(u_12 - (v_1 * qq)) + u_3) )
    BEGIN
      qq--;
      if (qq < 0)
      BEGIN                       /* we failed! */
	return(RT_ERROR);
      END
    END

#ifdef TRACE_DIV
  printf(" tuned qq is: %ld\n", qq);
#endif

  /* STEP D4, Mulitply and subtract */
				  /* convert qq to a byte array */
    qq_field I_ BYTE_ARRAY_SIZE -2 _I = qq;
    qq_field I_ sign_offset _I = 1;   /* pos. number */

    if (qq == 0)                  /* optimize if can            */
    BEGIN                         /* zero times ? is zero!      */
      memcpy(r_field, qq_field, sizeof(r_field));
    END
    else
    BEGIN
      status = rt_mula(r_field, v_field, qq_field);
      if (status NE RT_OK)
      BEGIN
	return(status);
      END
    END
#ifdef TRACE_DIV
  printf(" r_field is: "); SHOW_BA(r_field);
#endif
				  /* align the subtraction      */
    if (qq NE 0)
    BEGIN
      r_shift = ((BYTE_ARRAY_SIZE - 2) -j -n);
      rt_shta(r_field, r_shift);

#ifdef TRACE_DIV
  printf(" shift value is: %d\n", r_shift);
  printf(" r_field shifted is: "); SHOW_BA(r_field);
#endif
      status = rt_suba(u_field, u_field, r_field);
      if (status NE RT_OK)
      BEGIN
	return(status);
      END
    END /* of qq NE 0 */

#ifdef TRACE_DIV
  printf(" u_field - r_field is: "); SHOW_BA(u_field);
#endif

  /* STEP D5, Test remainder */
    c_field I_ j  _I = qq;     /* <==== see this             */

#ifdef TRACE_DIV
  printf(" c_field is: "); SHOW_BA(c_field);
#endif

  /* STEP D6, Add back, neg result */
     if (u_field I_ sign_offset _I NE 1)
     BEGIN /* we tried too hard, must add back 1 shifted v_field! */
	   /* use r_field as work field!                          */
       c_field I_ j _I--;
       memcpy(r_field,v_field,sizeof(r_field));
       rt_shta(r_field, r_shift);
       status = rt_adda(u_field, u_field, r_field);
       if (status NE RT_OK)
       BEGIN
	 return(status);
       END
#ifdef TRACE_DIV
       printf("u_field neg., so added back 1 v_field (shifted)!\n");
#endif
     END

#ifdef TRACE_DIV
  printf("           u_field is: "); SHOW_BA(u_field);
  printf("           v_field is: "); SHOW_BA(v_field);
  printf(" corrected c_field is: "); SHOW_BA(c_field);
  printf("dec. places in c_field: %ld\n", n);
#endif

  /* STEP D7, Loop on j - really end loop */

  END /* of while on j <= m */

  /* STEP D8, Unnormalize - realy do nothing since we can ignore the */
			    /* remainder                             */

  /* LAST, process the sign and shift */
				  /* NOT u_field and v_field!   */
  c_field I_ sign_offset _I = a_field I_ sign_offset _I *
			      b_field I_ sign_offset _I;

  *dec_places_ptr = n;            /* as a side effect, n is also*/
				  /* the number of dec. places  */
				  /* in the c_field array       */

  return(RT_OK);

END

/*********************************************************************
***********************************************************************
*                                                                     *
*         End of Module rtpack.c                                      *
*                                                                     *
***********************************************************************
**********************************************************************/

