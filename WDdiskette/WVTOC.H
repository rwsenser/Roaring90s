/* WVTOC.H: this include file contains the structures for the
 *          for the WANG floppy disk VTOC.
 *
 */
 
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


				  /* VOL1 structure             */
struct vol1 {
  unsigned char vol1_junk1  [ 0x100 ];
  unsigned char vol1_lit    [ 0x4   ];
  unsigned char vol1_volume [ 0x6   ];
  unsigned char vol1_junk2  [ 0x4e  ];
  unsigned char vol1_credate [ 0x3   ];
  unsigned char vol1_x1strt [ 0x3   ];
  unsigned char vol1_x1end  [ 0x3   ];
  unsigned char vol1_junk3  [ 0x6a4 ];

} ;

				  /* FDX1 structure             */
struct fdx1_entry {
  unsigned char fdx1_ename    [ 0x8   ];
  unsigned char fdx1_eblkx    [ 0x2   ];
  unsigned char fdx1_efiles   [ 0x2   ];
} ;

struct fdx1 {
  unsigned char fdx1_freecnt [ 0x2   ];
  unsigned char fdx1_freeblk [ 0x2   ];
  unsigned char fdx1_freex2  [ 0x2   ];
  unsigned char fdx1_ffeefd  [ 0x2   ];
  struct fdx1_entry fdx1_array [ 169 ];
  unsigned char fdx1_bspare1 [ 0x8   ];
  unsigned char fdx1_btyp ;
  unsigned char fdx1_spare2;
  unsigned char fdx1_blkchn  [ 0x2   ];
} ;

				  /* FDX2 structure             */
struct  fdx2_entry {
  unsigned char fdx2_ename   [ 0x8   ];
  unsigned char fdx2_eblkx   [ 0x2   ];
} ;

struct fdx2_record {
  unsigned char fdx2_ruse;
  unsigned char fdx2_rspare  [ 0x5   ];
  unsigned char fdx2_rname   [ 0x8   ];
  unsigned char fdx2_recchn  [ 0x2   ];
  struct fdx2_entry fdx2_earray [ 49 ];
} ;

struct fdx2 {
  struct fdx2_record fdx2_rarray [4] ;
  unsigned char fdx2_spare2  [ 0x14  ];
  unsigned char fdx2_btyp;
  unsigned char fdx2_bspare;
  unsigned char fdx2_blkchn  [ 0x2   ];
} ;

				  /* FDR1 structure             */
struct fdr1_entry {
  unsigned char fdr1_format;
  unsigned char fdr1_xtntcount;
  unsigned char fdr1_org;
  unsigned char fdr1_flags;
  unsigned char fdr1_x1ptr   [ 0x2   ];
  unsigned char fdr1_filename [ 0x8  ];
  unsigned char fdr1_filesection;
  unsigned char fdr1_credate [ 0x3   ];
  unsigned char fdr1_moddate [ 0x3   ];
  unsigned char fdr1_expdate [ 0x3   ];
  unsigned char fdr1_fpclass;
  unsigned char fdr1_creator [ 0x3   ];
  unsigned char fdr1_blksize [ 0x2   ];
  unsigned char fdr1_secext  [ 0x2   ];
  unsigned char fdr1_x1strt  [ 0x3   ];
  unsigned char fdr1_x1end   [ 0x3   ];
  unsigned char fdr1_x2strt  [ 0x3   ];
  unsigned char fdr1_x2end   [ 0x3   ];
  unsigned char fdr1_x3strt  [ 0x3   ];
  unsigned char fdr1_x3end   [ 0x3   ];
  unsigned char fdr1_spare2  [ 0x2   ];
  unsigned char fdr1_nrecs   [ 0x4   ];
  unsigned char fdr1_recsize [ 0x2   ];
  unsigned char fdr1_spare3;
  unsigned char fdr1_eblk    [ 0x3   ];
  unsigned char fdr1_erec    [ 0x2   ];
  unsigned char fdr1_spare4  [ 0xc   ];
  unsigned char fdr1_chain   [ 0x4   ];
} ;

struct fdr1 {
  struct fdr1_entry fdr1_array [25];
  unsigned char fdr_reccnt;
  unsigned char fdr_btyp;
  unsigned char fdr_bspare;
  unsigned char fdr1_blkchn  [ 0x2   ];
} ;


/* end of wvtoc.h */
