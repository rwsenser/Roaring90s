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
*           MODULE NAME : rt test pack #3 (rttstr.c)                  *
*                                                                     *
*          REVISION NO. :                                             *
*                                                                     *
*                AUTHOR : RW Senser                                   *
*                                                                     *
*                  DATE : 19.06.90                                    *
*                                                                     *
*           DESCRIPTION : Caluclator program to use the rt_pack       *
*                         routines.                                   *
*                                                                     *
*                                                                     *
*          TEST HISTORY :                                             *
*                                                                     *
*              MODIFIED :                                             *
*                                                                     *
*                                                                     *
***********************************************************************
**********************************************************************/

#include "rthead.h"

#define SCALE (2);

main()

BEGIN

  Short in_1_scale,  in_2_scale,  out_scale;
  Short in_1_length, in_2_length, out_length;

  Int i;

  Char in_1 I_ 16 _I;
  Char in_2 I_ 16 _I;
  Char out  I_ 16 _I;

  Char pack_in_1 I_ 8 _I;
  Char pack_in_2 I_ 8 _I;
  Char pack_out  I_ 8 _I;

  Char buffer I_ 80 _I;
  Char operator I_ 8 _I;

  Rt_pack_entry pack_block;

/**********************************************************************
*                                                                     *
*                    Code starts here                                 *
*                                                                     *
**********************************************************************/

                                  /* input first field          */
  printf("enter size of in_1 \n");
  gets(buffer);
#ifdef I370
  printf("%s\n", buffer);
#endif
  in_1_length = atoi(buffer);

  printf("enter scale of in_1 \n");
  gets(buffer);
#ifdef I370
  printf("%s\n", buffer);
#endif
  in_1_scale = atoi(buffer);

  printf("enter value of in_1 \n");
  gets(in_1);
#ifdef I370
  printf("%s\n", in_1);
#endif

  printf("enter operator \n");
  gets(operator);
#ifdef I370
  printf("%s\n", operator);
#endif

                                  /* input second field         */
  printf("enter size of in_2 \n");
  gets(buffer);
#ifdef I370
  printf("%s\n", buffer);
#endif
  in_2_length = atoi(buffer);

  printf("enter scale of in_2 \n");
  gets(buffer);
#ifdef I370
  printf("%s\n", buffer);
#endif
  in_2_scale = atoi(buffer);

  printf("enter value of in_2 \n");
  gets(in_2);
#ifdef I370
  printf("%s\n", in_2);
#endif
                                  /* input third field specs    */
  printf("enter size of out \n");
  gets(buffer);
#ifdef I370
  printf("%s\n", buffer);
#endif
  out_length = atoi(buffer);

  printf("enter scale of out \n");
  gets(buffer);
#ifdef I370
  printf("%s\n", buffer);
#endif
  out_scale = atoi(buffer);

                                  /* pack the first field       */

  pack_block.pck_func         = RT_DEC_DISP_PCK;

  pack_block.pck_from1_ptr    = in_1;
  pack_block.pck_from1_length = in_1_length;
  pack_block.pck_from1_scale  = in_1_scale;

  pack_block.pck_to_ptr       = pack_in_1;
  pack_block.pck_to_length    = sizeof(pack_in_1);
  pack_block.pck_to_scale     = SCALE;

  printf(" first pack: %d \n", rt_pckf(&pack_block));

                                  /* pack the second field      */

  pack_block.pck_func         = RT_DEC_DISP_PCK;

  pack_block.pck_from1_ptr    = in_2;
  pack_block.pck_from1_length = in_2_length;
  pack_block.pck_from1_scale  = in_2_scale;

  pack_block.pck_to_ptr       = pack_in_2;
  pack_block.pck_to_length    = sizeof(pack_in_2);
  pack_block.pck_to_scale     = SCALE;

  printf("second pack: %d \n", rt_pckf(&pack_block));

                                  /* request the packed op      */

  switch (operator I_ 0 _I)
  BEGIN
    case '+': pack_block.pck_func = RT_DEC_ADD;
              break;
    case '-': pack_block.pck_func = RT_DEC_SUB;
              break;
    case '*': pack_block.pck_func = RT_DEC_MUL;
              break;
    case '/': pack_block.pck_func = RT_DEC_DIV;
              break;
    case '?': pack_block.pck_func = RT_DEC_CMP;
              break;
  END

  pack_block.pck_from1_ptr    = pack_in_1;
  pack_block.pck_from1_length = sizeof(pack_in_1);
  pack_block.pck_from1_scale  = SCALE;

  pack_block.pck_from2_ptr    = pack_in_2;
  pack_block.pck_from2_length = sizeof(pack_in_2);
  pack_block.pck_from2_scale  = SCALE;

  pack_block.pck_to_ptr       = pack_out;
  pack_block.pck_to_length    = sizeof(pack_out);
  pack_block.pck_to_scale     = out_scale;

  printf("operation: %d \n", rt_pckf(&pack_block));

                                  /* unpack the result field    */

  pack_block.pck_func         = RT_DEC_PCK_DISP;

  pack_block.pck_from1_ptr    = pack_out;
  pack_block.pck_from1_length = sizeof(pack_out);
  pack_block.pck_from1_scale  = out_scale;

  pack_block.pck_to_ptr       = out;
  pack_block.pck_to_length    = out_length;
  pack_block.pck_to_scale     = out_scale;

  printf(" unpack: %d \n", rt_pckf(&pack_block));

  printf(" result: ");
  for (i=0; i< out_length; i++)
  BEGIN
    printf("%c",*(out+i));
  END
  printf("\n");

  return(0);

END

/**********************************************************************
***********************************************************************
*                                                                     *
*         End of Module rttstp.c                                      *
*                                                                     *
***********************************************************************
**********************************************************************/
