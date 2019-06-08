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
*         Macro Definitions                                           *
*                                                                     *
***********************************************************************
**********************************************************************/

                                 /* alignment insensitive moves */
#define RT_MOVE_TO(to,from)   memcpy(&to,from,sizeof(to))
#define RT_MOVE_FROM(to,from) memcpy(to,&from,sizeof(from))


                                  /* C packed support macros    */

#define RT_SUNP_POS    (0xc0)     /* unpacked positive          */
#define RT_SUNP_NEG    (0xd0)     /* unpacked negative          */
                                  /* unpacked un-signed         */
#define RT_SUNP_USN    ('0')      /* 0xf0 for ibm 370           */

                                  /* packed positive            */
#define RT_SPCK_POS    (RT_SUNP_POS >> 4)
                                  /* packed negative            */
#define RT_SPCK_NEG    (RT_SUNP_NEG >> 4)
                                  /* packed un-signed           */
#define RT_SPCK_USN    (RT_SUNP_POS >> 4)

#define RT_SPCK_MAX    (8)        /* max packed field in bytes  */
#define RT_SUNP_MAX    (15)       /* max unpacked field (bytes) */
#define RT_SUNP_BUF    (16)       /* rt_sunp-max + 1            */

#define RT_SUNP_MATH_MAX (9)      /* this is the max number     */
                                  /* of significant digits that */
                                  /* can be put into an integer */

#define RT_DEC_WORK_SIZE    (120) /* work area size (in bytes)  */

#define RT_DEC_LT            (1)  /* less than code             */
#define RT_DEC_EQ            (2)  /* equal code                 */
#define RT_DEC_GT            (3)  /* greater than code          */

#define RT_DEC_ADD        (0x04)  /* add operation request      */
#define RT_DEC_SUB        (0x08)  /* subtract operation request */
#define RT_DEC_MUL        (0x0c)  /* multiply operation request */
#define RT_DEC_DIV        (0x10)  /* divide operation request   */

#define RT_DEC_CMP        (0x14)  /* packed compare request     */

#define RT_DEC_PCK_PCK    (0x18)  /* convert packed to packed   */
#define RT_DEC_PCK_DISP   (0x1c)  /* convert packed to display  */
                                  /* display = PIC 99999 etc.   */
#define RT_DEC_DISP_PCK   (0x20)  /* convert display to packed  */

/**********************************************************************
***********************************************************************
*                                                                     *
*         Structure and Type Definitions                              *
*                                                                     *
***********************************************************************
**********************************************************************/

/*page*/
/**********************************************************************
*                                                                     *
*                             P A C K                                 *
*                                                                     *
* This struct describes the information that is passed to the packed  *
* decimal routines.  If this sturct is changed, then it is likely     *
* the corresponding assembler (or whatever lanaguage) control block   *
* must be changed.  On the 370, this is an assembler routine.         *
*                                                                     *
**********************************************************************/

typedef struct
BEGIN
  Long  pck_func             ;    /* function code, see RT_PCK  */
                                  /* codes.                     */

  Byte  *pck_to_ptr          ;    /* pointer to "to" field      */
  Short pck_to_length        ;    /* length in bytes            */
  Short pck_to_scale         ;    /* scale in digits            */

  Byte  *pck_from1_ptr       ;    /* pointer to "from1" field   */
  Short pck_from1_length     ;    /* length in bytes            */
  Short pck_from1_scale      ;    /* scale in digits            */

  Byte  *pck_from2_ptr       ;    /* pointer to "from2" field   */
  Short pck_from2_length     ;    /* length in bytes            */
  Short pck_from2_scale      ;    /* scale in digits            */

  Byte  pck_work_area I_ RT_DEC_WORK_SIZE _I;
                                  /* workarea for use by packed */
                                  /* routine, 370 version uses  */
                                  /* this area.                 */

END
  Rt_pack_entry;


/**********************************************************************
***********************************************************************
*                                                                     *
*         External Data Definitions                                   *
*                                                                     *
***********************************************************************
**********************************************************************/


/**********************************************************************
***********************************************************************
*                                                                     *
*         External Function Definitions                               *
*                                                                     *
***********************************************************************
**********************************************************************/


EXPORT Int rt_pack(
#ifdef PROTOTYPING

  Rt_pack_entry *pack_ptr

#endif
                    );        /* 370 like pack operation       */



EXPORT Int rt_unpk(
#ifdef PROTOTYPING

  Rt_pack_entry *pack_ptr

#endif
                    );        /* 370 like unpack operation     */



			      /*RS*/
			      /* removed rt_cvbp() and          */
			      /* rt_cvdp()                      */


EXPORT Int rt_zapp(
#ifdef PROTOTYPING

  Rt_pack_entry *pack_ptr

#endif
                    );        /* 370 like zero and add packed  */



EXPORT Int rt_srpp(
#ifdef PROTOTYPING

  Rt_pack_entry *pack_ptr,
  Int            shift_factor,
  Int            round_factor

#endif
                    );        /* 370 like shift and round      */



EXPORT Int rt_cpp(
#ifdef PROTOTYPING

  Rt_pack_entry *pack_ptr

#endif
                    );         /* 370 like compare packed       */



EXPORT Int rt_mthp(
#ifdef PROTOTYPING

  Rt_pack_entry *pack_ptr,
  Char          operation

#endif
                    );        /* 370 like packed math          */

EXPORT Int rt_cvap(               /*RS*/
#ifdef PROTOTYPING

  Rt_pack_entry *pack_ptr

#endif
		    );


/**********************************************************************
***********************************************************************
*                                                                     *
*         End of Module Export Definition File                        *
*                                                                     *
***********************************************************************
**********************************************************************/
