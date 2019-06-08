/* WD.C: uses low-level disk access functions
 *       to process wang diskettes on the pc. Can read
 *       files from the WANG floppy disk and
 *       to copy them to a PC files.
 *
 *    uses:  _bios_disk
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

#include <stdio.h>
#include <ctype.h>
#include <conio.h>
#include <bios.h>
#include <dos.h>
#include <stdlib.h>

#include "wvtoc.h"                /* WANG VTOC structures       */

				  /* wang int --> pc int        */
#define INT(i)  (i[0]*256 + i[1])
#define INT3(i) (i[0]*256*256+i[1]*256+i[2])
#define INT4(i) (i[0]*256*256*256+i[1]*256*256+i[2]*256+i[3])

void wang_copy();
void scan_fdx1();
void scan_fdx2();
void process_one_lib();
void process_one_file();
void get_name();
void wang_block();
void read_wang_block();
void read_pc_block();

unsigned char far diskbuf[2048];
unsigned char far fdx1buf[2048];
unsigned char far fdx2buf[2048];
unsigned char far fdr1buf[2048];

#define OUT_RECORD_SIZE (2048)
unsigned char far out_record[OUT_RECORD_SIZE];

struct f_name {
unsigned char file_name[8];
};

#define FILE_ARRAY_SIZE (200)
struct f_name file_array[FILE_ARRAY_SIZE];

static unsigned file_count;


main( int argc, char *argv[] )
{

    unsigned sector_size = 2048;
    unsigned char far *p, linebuf[17];
    unsigned block, record, c, i, j;
    char drive;
    char command[80];
    char numb[80];
    char library[80];

    struct vol1 *vol1_ptr;
				  /* set default diskette drive */
    if( argc != 2 )
      {
       drive = 'a';
      }
    else
      {
       drive = argv[1][0];
      }

    command[0] = ' ';
    while (command[0] != '.')
      {
       printf("\n WD options: ('v': volume, 'l': libraries,");
       printf(" 'f': files, 'd': dump, '.': exit)\n");
       printf(": ");
       gets(command);
       switch (command[0])
	{
				  /* list volume info           */
	 case 'v':
	   block = 0;
	   read_wang_block(drive, block, diskbuf);
	   vol1_ptr = ( struct vol1 * ) diskbuf;
	   printf(" Volume: %6.6s \n", vol1_ptr -> vol1_volume);
	   break;

				  /* list libraries             */
	 case 'l':
				  /* scan fdx1 and print lib    */
				  /* names                      */
	   printf("\n");
	   scan_fdx1( drive, 'P', library);
	   break;
				  /* list files                 */
	 case 'f':
				  /* scan fdx1 to find lib      */
				  /* list files in lib and then */
				  /* copy file to pc (option)   */
	   printf("WANG library name: ");
	   get_name(library);
	   if (library[0] != '.')
	     {
	      scan_fdx1( drive, 'F', library);
	     }
	   break;

				  /* dump wang block in buffer  */
	 case 'd':
	   printf("block number: (0=vol1, 1=fdav, 2=fdx1, 3=fdx2): ");
	   gets(numb);
	   if (numb[0] != '.')
	     {
	      block=atoi(numb);
	      read_wang_block(drive, block, diskbuf);
	      c = 0;
	      j = 0;
	      printf ("%.4x ", c);
	      for( p = diskbuf, i = 0;
		   p < (diskbuf + sector_size);
		   p++ )
		{
		 linebuf[i++] = (isgraph(*p) != 0) ? *p : '.';
		 c++;
		 printf( "%.2x ", *p );
		 if( i == 16 )
		   {
		    linebuf[i] = '\0';
		    printf( " %16s\n", linebuf );
		    j++;
		    if (j > 22)
		      {
		       printf("push enter");
		       gets(numb);
		       if (numb[0] == '.')
			 {
			  break;
			 }
		       j = 0;
		      }
		    printf("%.4x ", c);
		    i = 0;
		   }
		}
	     }
	   break;
				  /* exit                       */
	 case '.':
	   printf(" WD exit. \n");
	   break;
				  /* error                      */
	 default:
	   printf(" what was that?? \n");
	   break;
	}
      }

    exit(0);

}

/* this routine handles the actual coping of the wang file to
 * a PC file.
 */

void wang_copy(fdr1_entry_ptr, drive, pc_file)

  struct fdr1_entry *fdr1_entry_ptr;
  char drive;
  char *pc_file;

  {

   unsigned start1, end1;
   unsigned records;
   unsigned ch_offset;
   unsigned lrecl;
   unsigned rec_length;
   unsigned blksize;
   unsigned blk_length;
   unsigned char *blk_ptr;
   char status;
   unsigned recs_in_block;
   unsigned end_block_cnt;
   unsigned i, j, k;
   unsigned block;
   unsigned org;
   unsigned flags;
   char stuff[20];

   FILE *fopen(), *fp;

   printf("\n");
   printf("Copying %8.8s ", fdr1_entry_ptr -> fdr1_filename);
   printf("to PC file %s. \n", pc_file);

   org   = fdr1_entry_ptr -> fdr1_org;
   flags = fdr1_entry_ptr -> fdr1_flags;

   if (org != 0x01 && org != 0x21 && org != 0x61)
     {
      printf("\n");
      printf("error - fdr1_org must be fixed consecutive, or\n");
      printf("        variable consecutive compressed, or");
      printf("        variable print compressed");
      return;
     }

   if ((org == 0x21 || org == 0x61) && ((flags & 0x40) != 0x40))
     {
      printf("\n");
      printf("error - variable file must be compressed\n");
      return;
     }

   if (fdr1_entry_ptr -> fdr1_xtntcount > 1)
     {
      printf("\n");
      printf("error - fdr1_xtntcount must be 1\n");
      printf("(too many extents)\n");
      return;
     }

   fp = fopen(pc_file, "w");
   if (fp == NULL)
     {
      printf("\n");
      printf("error -  %s will not open.", pc_file);
      return;
     }

   start1 = INT3(fdr1_entry_ptr -> fdr1_x1strt);
   end1   = INT3(fdr1_entry_ptr -> fdr1_x1end) - 1;

   end_block_cnt = INT(fdr1_entry_ptr -> fdr1_erec);

   lrecl   = INT(fdr1_entry_ptr -> fdr1_recsize);
   blksize = INT(fdr1_entry_ptr -> fdr1_blksize);

   recs_in_block = blksize / lrecl;

   records = 0;

   for (block = start1; block <= end1; block++)
     {
      read_wang_block(drive, block, diskbuf);
				  /* for fixed consecutive */
      if (org == 0x01)
	{
	 for (i=0; i < recs_in_block; i++)
	   {
	    if (block == end1 && i == end_block_cnt)
	      {
	       break;
	      }
	    for (j=(lrecl-1); j > 0; j--)
	      {                   /* trim trailing blanks       */
	       rec_length = j + 1;
	       if (diskbuf[((i*lrecl)+j)] != ' ')
		 {
		  break;
		 }
	      }
	    for (j=0; j < rec_length; j++)
	      {
	       fprintf(fp, "%c", diskbuf[((i*lrecl)+j)] );
	      }
	    fprintf(fp, "\n");
	    records++;
	   }
	}
      else
				  /* consecutive variable and   */
				  /* compressed                 */
	{
	 blk_ptr = diskbuf;
	 blk_length = INT(blk_ptr);
	 blk_ptr++;
	 blk_ptr++;
	 while (blk_ptr < (diskbuf + blk_length))
	   {
	    rec_length = INT(blk_ptr) - 2;
	    blk_ptr++;
	    blk_ptr++;
	    ch_offset = 0;
	    status = 'S';
	    for (i = 0; i < rec_length; i++)
	      {
	       switch (status)
		 {
		  case 'S':        /* scan for control info      */
		    if ((*blk_ptr & 0x80) == 0x80)
				  /* compressed area found      */
		      {
		       status = 'C';
		       k = (*blk_ptr & 0x7f) + 1;
		      }
		    else
				  /* non compressed area found */
		      {
		       status = 'N';
		       k = (*blk_ptr) + 1;
		      }
		    break;
		  case 'C':
		    for (j=0; j<k; j++)
		      {
		       out_record[ch_offset] = *blk_ptr;
		       ch_offset++;
		      }
		    status = 'S';
		    break;
		  case 'N':
		    out_record[ch_offset] = *blk_ptr;
		    ch_offset++;
		    k--;
		    if (k == 0)
		      {
		       status = 'S';
		      }
		    break;
		 }  /* of switch */
	       blk_ptr++;
	      }  /* of for */
	    rec_length = ch_offset;
	    for (j=(ch_offset-1); j > 0; j--)
	      {                   /* trim trailing blanks       */
	       rec_length = j + 1;
	       if (out_record[j] != ' ')
		 {
		  break;
		 }
	      }
	    for (j=0; j < rec_length; j++)
	      {
	       if (org == 0x61)
		 {
		  if (out_record[j] == 0x80)
		    {
		     out_record[j] = 0x0c;
		    }
		  else if (isgraph(out_record[j]) == 0)
		    {
		     out_record[j] = ' ';
		    }
		 }
	       fprintf(fp, "%c", out_record[j] );
	      }
	    records++;
	    fprintf(fp,"\n");
	   }  /* of while */
	}
     }

     fclose(fp);
     printf("records copied: %d\n", records);

   return;
  }


/* this routine scans the fdx1 index block and passes each
 * entry to the requested function.
 */
void scan_fdx1(drive, function, library)

  char drive;
  char function;
  char *library;

  {
   unsigned block = 2;
   unsigned i;
   unsigned chain;
   unsigned key = 999;

   struct fdx1 *fdx1_ptr;
   struct fdx1_entry *fdx1_entry_ptr;

   read_wang_block(drive, block, fdx1buf);

   fdx1_ptr = (struct fdx1 *) fdx1buf;
   fdx1_entry_ptr = fdx1_ptr -> fdx1_array;
   chain = INT(fdx1_ptr -> fdx1_blkchn);
   for ( i=0; i < 169; i++)
     {
      if (fdx1_entry_ptr -> fdx1_ename [0] != '\0')
	{
	 if (function == 'P')
	   {
	    printf(" %8.8s ", fdx1_entry_ptr -> fdx1_ename);
	    printf(" %.4d\n", INT(fdx1_entry_ptr -> fdx1_efiles));
	   }
	 else if (function == 'F')
	   {
	    key = i + 169;
	    process_one_lib(fdx1_entry_ptr, library, key, drive);
	   }
	}
      fdx1_entry_ptr++;
     }
    if (chain != 0)
      {
       printf(" only first 169 libraries processed!\n");
      }

    return;

   }

/* this routine scans the fdx2 index blocks and passes each
 * entry to the requested function.
 */
void scan_fdx2(drive, block, record, function, file, key)

  char drive;
  unsigned block;
  unsigned record;
  char function;
  char *file;
  unsigned key;

  {
   unsigned i;
   unsigned chain;

   struct fdx2 *fdx2_ptr;
   struct fdx2_record *fdx2_record_ptr;
   struct fdx2_entry  *fdx2_entry_ptr;

   read_wang_block(drive, block, fdx2buf);
   fdx2_ptr = (struct fdx2 *) fdx2buf;
   fdx2_record_ptr = &(fdx2_ptr -> fdx2_rarray[record]);

   chain = INT(fdx2_record_ptr -> fdx2_recchn);

   for ( i=0; i < 49; i++)
     {
      if (fdx2_record_ptr -> fdx2_earray[i].fdx2_ename [0] != '\0')
	{
	 if (function == 'P')
	   {
	    printf(" %8.8s \n",
		    fdx2_record_ptr -> fdx2_earray[i].fdx2_ename);
	    if (file_count < FILE_ARRAY_SIZE)
	      {
	       memcpy(file_array[file_count].file_name,
		      fdx2_record_ptr -> fdx2_earray[i].
					 fdx2_ename,
		      sizeof(file_array[file_count].file_name));
	      }
	    file_count++;
	   }
	 else if (function == 'F')
	   {
	    fdx2_entry_ptr = &(fdx2_record_ptr -> fdx2_earray[i]);
	    process_one_file(fdx2_entry_ptr, file, key, drive);
	   }
	}
     }

   if (chain != 0)
     {                            /* chain to next fdx2 block   */
      block = chain / 4 + 1;
      record = chain % 4;
      scan_fdx2(drive, block, record, function, file);
     }

   return;

   }

/* this routine processes one WANG library, it lists the files
 * and then allows the user to process one file
 */
void process_one_lib( fdx1_entry_ptr, library, key, drive)

  struct fdx1_entry *fdx1_entry_ptr;
  char *library;
  unsigned key;
  char drive;

  {
   unsigned block, record;
   unsigned i;

   char file[80];

   if (memcmp(library, fdx1_entry_ptr -> fdx1_ename,
	      sizeof(fdx1_entry_ptr -> fdx1_ename) ) == 0)
     {
      block = (INT(fdx1_entry_ptr -> fdx1_eblkx) / 4) + 1;
      record = INT(fdx1_entry_ptr -> fdx1_eblkx) % 4;
      file_count = 0;
      printf("\n");
      scan_fdx2(drive, block, record, 'P', file);
      printf("Number of files: %d\n", file_count);
      printf("\n");
      printf("To copy file to PC enter WANG file name, or enter a\n");
      printf("'*' to copy entire library, or enter '.' to skip copy.\n");

      printf("WANG file name: ");
      get_name(file);

      if (file[0] == '.')
	{
	 return;
	}
      else if (file[0] == '*')
	{
	 for (i=0; i< file_count; i++)
	   {
	    memcpy(file, file_array[i].file_name, sizeof(file));
	    printf("\n");
	    printf("WANG file name is: %.8s\n", file);
	    scan_fdx2(drive, block, record, 'F', file, key);
	   }
	}
      else
	{
	 scan_fdx2(drive, block, record, 'F', file, key);
	}
     }

   return;
  }

/* this routine processes one file, the user may request the
 * file be copied to a pc file.
 */
void process_one_file( fdx2_entry_ptr, file, key, drive)

  struct fdx2_entry *fdx2_entry_ptr;
  char *file;
  unsigned key;
  char drive;

  {
   unsigned i;
   unsigned block;
   char pc_file[80];

   struct fdr1 *fdr1_ptr;
   struct fdr1_entry *fdr1_entry_ptr;

   if (memcmp(file,fdx2_entry_ptr -> fdx2_ename,
	      sizeof(fdx2_entry_ptr -> fdx2_ename)) == 0)
     {
      block = INT(fdx2_entry_ptr -> fdx2_eblkx) + 1;
      read_wang_block(drive, block, fdr1buf);
      fdr1_ptr = (struct fdr1 *) fdr1buf;
      for (i=0; i < 25; i++)
	{
	 if (fdr1_ptr -> fdr1_array[i].fdr1_format == '1')
	   {
	    if ((memcmp(fdr1_ptr -> fdr1_array[i].fdr1_filename,
		       file,8) == 0) &&
	       (INT(fdr1_ptr -> fdr1_array[i].fdr1_x1ptr) == key))
	      {
	       fdr1_entry_ptr = &(fdr1_ptr -> fdr1_array[i]);
	       printf("\n");
	       printf("'to' PC file name: ");
	       gets(pc_file);
	       if (pc_file[0] != '.')
		 {
		  wang_copy(fdr1_entry_ptr, drive, pc_file);
		 }
	      }
	   }
	}
     }
   return;
  }

/* this routine inputs WANG library and file names and
 * converts then to upper case and removes the \0.
 */

void get_name( field )

  char *field;

  {
   unsigned i;

   memset(field,' ',8);

   gets(field);

   for (i=0; i<8; i++)
     {
      if (field[i] == '\0')
	{
	 field[i] = ' ';
	}
      field[i] = toupper(field[i]);
     }
   return;
  }

/*  this routine converts the a wang diskette block number to
 *  the correct head, track, and sector
 */

#define HEADS (2)
#define SECTORS (15)

void wang_block(block, head, track, sector)

  unsigned *block;
  unsigned *head;
  unsigned *track;
  unsigned *sector;

  {
   unsigned work_value;

   work_value = *block;

   /* get the sector number */
   *sector = work_value % SECTORS + 1;
   work_value = work_value / SECTORS;

   /* get the head number   */
   *head = work_value % HEADS;
   work_value = work_value / HEADS;

   /* get the track number */
   *track = work_value;

   return;
  }

/* this routine reads one "wang block" which is 2k long
 * from the floppy drive.
 */
void read_wang_block(drive, block_id, block_ptr)

  char     drive;
  unsigned block_id;
  char     *block_ptr;

  {

   unsigned internal_block_id;
   unsigned pc_sector_size = 512;
   char *ptr;

				  /* convert to pc block number */
    internal_block_id = (block_id  * 4);
    ptr = block_ptr;

				  /* read 1rst pc sector        */
    read_pc_block(drive, internal_block_id, ptr);

				  /* read 2nd pc sector         */
    internal_block_id++;
    ptr += pc_sector_size;
    read_pc_block(drive, internal_block_id, ptr);

				  /* read 3rd pc sector         */
    internal_block_id++;
    ptr += pc_sector_size;
    read_pc_block(drive, internal_block_id, ptr);

				  /* read 4th pc sector         */
    internal_block_id++;
    ptr += pc_sector_size;
    read_pc_block(drive, internal_block_id, ptr);

    return;
  }

/* this routine reads one "pc block" which is 512 bytes long
 * from the floppy drive.
 */
void read_pc_block(drive, pc_block_id, block_ptr)

  char     drive;
  unsigned pc_block_id;
  char     *block_ptr;

  {
    unsigned sector_size = 512;

    unsigned head, track, sector, retry_track;
    unsigned status = 0, i, c;
    struct diskinfo_t di;

    if( (di.drive = toupper( drive ) - 'A' ) > 1 )
    {
	printf( "Must be floppy drive" );
	exit( 1 );
    }


    wang_block(&pc_block_id, &head, &track, &sector);

    di.head     = head;
    di.track    = track;
    di.sector   = sector;
    di.nsectors = 1;
    di.buffer   = block_ptr;


    /* Try reading disk three times before giving up. */
    for( i = 0; i < 3; i++ )
    {
	status = _bios_disk( _DISK_READ, &di ) >> 8;
	if( !status )
	    break;
	if(i == 1)
	  {
	   retry_track = di.track;
	   di.track = 0;
	   _bios_disk( _DISK_READ, &di);   /* move to track 0 */
	   di.track = retry_track;
	  }
    }
    if( status )
      {
       printf( "Error: 0x%.2x\n", status );
       exit(1);
      }

   return;
  }



