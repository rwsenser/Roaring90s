/* WDMAKE.C: uses low-level disk access functions
 *           to write to wang diskettes on the pc. Can copy
 *           PC text (source) files to the WANG floppy disk.
 *
 *    limits: creates a library called PCLIB on the WANG diskette
 *            and a maximum of 196 files can be put in this one
 *            library (less if the VTOC is small).  The files to
 *            be copied to the PC are ......
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

				  /* pc int --> wang int        */
#define TO_INT(i,j) memcpy(((char *) i),((char *) &j)+1,1); \
		    memcpy(((char *) i)+1,((char *) &j),1);

#define TO_INT3(i,j) memset(((char *) i),0,1); \
		     memcpy(((char *) i)+1,((char *) &j)+1,1); \
		     memcpy(((char *) i)+2,((char *) &j),1);


#define TO_INT4(i,j)  memset(((char *) i),0,2); \
		      memcpy(((char *) i)+2,((char *) &j)+1,1); \
		      memcpy(((char *) i)+3,((char *) &j),1);


#define PCLIB "PCLIB   "

void process_fdav();
void process_fdx1();
void process_fdx2();
void process_fdr1();
unsigned process_data();
void wang_copy();
void get_name();
void wang_block();
void read_wang_block();
void write_wang_block();
void read_pc_block();
void write_pc_block();

unsigned char far diskbuf[2048];
unsigned char far fdx1buf[2048];
unsigned char far fdx2buf[2048];
unsigned char far fdr1buf[2048];

struct f_name {
unsigned char file_name[12];      /* WANG filename              */
unsigned file_start;
unsigned file_end;
unsigned file_block_last;
unsigned file_fdr_block;
unsigned file_records;
};

#define FILE_ARRAY_SIZE (196)
struct f_name file_array[FILE_ARRAY_SIZE];

static unsigned file_count;

main( int argc, char *argv[] )
{

    unsigned sector_size = 2048;
    unsigned lrecl;
    unsigned char far *p, linebuf[17];
    unsigned block, c, i, j;
    unsigned start_vtoc_block, end_vtoc_block;
    unsigned max_number_files;
    unsigned free_block;
    unsigned fdr_block;
    char drive;
    char error;
    char command[80];
    char numb[80];
    char file[80];
    char pc_file[80];

    struct vol1 *vol1_ptr;

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
       printf("\n WDMAKE options: ('v': volume, 'c': copy,");
       printf(" 'd': dump, '.': exit)\n");
       printf(": ");
       gets(command);
       switch (command[0])
	{
				  /* list volume info           */
	 case 'v':
	   block = 0;
	   read_wang_block(drive, block, diskbuf);
	   vol1_ptr = ( struct vol1 * ) diskbuf;
	   if (memcmp(vol1_ptr -> vol1_lit, "VOL1", 4) != 0)
	     {
	      printf(" Volume not a WANG diskette!\n");
	     }
	   else
	     {
	      printf(" Volume: %6.6s \n", vol1_ptr -> vol1_volume);
	     }
	   break;

				  /* copy files                 */
	 case 'c':
	   file_count = 0;
	   block = 0;
	   read_wang_block(drive, block, diskbuf);
	   vol1_ptr = ( struct vol1 * ) diskbuf;
	   if (memcmp(vol1_ptr -> vol1_lit, "VOL1", 4) != 0)
	     {
	      printf(" Volume not a WANG diskette!\n");
	      break;
	     }
	    printf("\n");
	    printf("The PC files in the current sub-directory will\n");
	    printf("be copied to the WANG ");
	    printf("volume %6.6s", vol1_ptr -> vol1_volume);
	    printf(" in library %s.\n", PCLIB);
	    printf("\n");
	    printf("All libraries and files that are currently on \n");
	    printf("this WANG volume will be totally DESTROYED.\n");
	    printf("\n");
	    printf("To continue enter 'X': ");
	    gets(numb);
	    if (numb[0] != 'X')
	      {
	       printf("copy aborted!\n");
	       break;
	      }
	   start_vtoc_block = INT3(vol1_ptr -> vol1_x1strt);
	   end_vtoc_block   = INT3(vol1_ptr -> vol1_x1end) - 1;
	   free_block = end_vtoc_block + 1;
	   max_number_files = ((end_vtoc_block - start_vtoc_block)
			       - 3) * 25;
	   if (max_number_files > 196)
	     {
	      max_number_files = 196;
	     }
	   printf("\n");
	   printf("This WANG volume can hold %d files.\n",
		   max_number_files);

	   for (i=0; i < max_number_files; i++)
	     {
	      printf("\n");
	      printf("(%3.3d) PC file name ('.' to exit): ", i);
	      gets(pc_file);
	      if (pc_file[0] == '.')
		{
		 break;
		}
	      error = 'Y';
	      while (error == 'Y')
		{
		 printf("    WANG file name: ");
		 get_name(file);
		 error = 'N';
		 if (i > 0)
		   {
		    for (j=0; j < i; j++)
		      {
		       if (memcmp(
			   file_array[j].file_name,
			   file,
			   sizeof(file_array[j].file_name)
				  ) == 0)
			 {
			  error = 'Y';
			  printf("error - duplicate WANG name\n");
			  break;
			 }
		      }
		   }
		}
	      memcpy(file_array[i].file_name,file,
		     sizeof(file_array[i].file_name));
	      file_count = i;
	      wang_copy(pc_file, &file_array[i], drive, &free_block);
	     }

				  /* build the fdr1 blocks that */
				  /* describe each of the files */
	   fdr_block = 4;
	   lrecl = 80;

	   process_fdr1(drive, lrecl, sector_size, 0, fdr_block);

				  /* build the fdx2 index       */
				  /* entries                    */
	   process_fdx2(drive,sector_size);

				  /* set the lib name and       */
				  /* set vtoc free space to     */
				  /* empty                      */
	   process_fdx1(drive);

				  /* set data area free space   */
				  /* to empty                   */
	   process_fdav(drive);


	   printf("\n");
	   printf("Copy operation completed. \n");
	   break;


				  /* list files                 */
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

/* this routine processes the FDAV block by making all of the
 * free space entries into binary zeros.
 */

void process_fdav(drive)

  char drive;

  {
   unsigned block = 1;
   unsigned char *fdav_ptr;

   read_wang_block(drive, block, diskbuf);
   fdav_ptr = diskbuf;
   memset(fdav_ptr,0,0x7fc);
   write_wang_block(drive, block, fdav_ptr);

   return;

  }

/* this routine processes the fdx1 index block and setups one
 * library on the WANG diskette called PCLIB.
 */
void process_fdx1(drive)

  char drive;

  {
   unsigned block = 2;
   unsigned fdx2_block =  8;     /* (3-1) * 4     */
   unsigned key = 999;
   unsigned count;

   struct fdx1 *fdx1_ptr;
   struct fdx1_entry *fdx1_entry_ptr;

   read_wang_block(drive, block, fdx1buf);

   fdx1_ptr = (struct fdx1 *) fdx1buf;
   memset(fdx1_ptr,0,0x7fc);

   fdx1_entry_ptr = fdx1_ptr -> fdx1_array;
   memset(fdx1_entry_ptr -> fdx1_ename,' ',
	  sizeof(fdx1_entry_ptr -> fdx1_ename));
   memcpy(fdx1_entry_ptr -> fdx1_ename,PCLIB,
	  (sizeof(PCLIB)-1));
   TO_INT(fdx1_entry_ptr -> fdx1_eblkx, fdx2_block);
   count = file_count + 1;
   TO_INT(fdx1_entry_ptr -> fdx1_efiles, count);

   write_wang_block(drive, block, fdx1_ptr);

   return;

   }

/* this routine process the file_array and produces the one
 * fdx2 block that lists the files in the one libaray.
 */
void process_fdx2(drive, blksize)

  char drive;
  unsigned blksize;

  {
   unsigned i;
   unsigned j;
   unsigned k;
   unsigned chain;

   struct fdx2 *fdx2_ptr;
   struct fdx2_record *fdx2_record_ptr;
   struct fdx2_entry  *fdx2_entry_ptr;

   unsigned block = 3;

   memset(fdx2buf,0,blksize);
   fdx2_ptr = (struct fdx2 *) fdx2buf;
   fdx2_ptr -> fdx2_btyp = '2';
   fdx2_ptr -> fdx2_rarray[0].fdx2_ruse = 'N';
   fdx2_ptr -> fdx2_rarray[1].fdx2_ruse = 'N';
   fdx2_ptr -> fdx2_rarray[2].fdx2_ruse = 'N';
   fdx2_ptr -> fdx2_rarray[3].fdx2_ruse = 'N';

   k = 0;

   fdx2_record_ptr = (struct fdx2_record *) fdx2buf;

   for (i = 0; i < 4; i++)
     {
      fdx2_entry_ptr = &(fdx2_record_ptr -> fdx2_earray[i]);
      fdx2_record_ptr -> fdx2_ruse = 'I';
      memset(fdx2_record_ptr -> fdx2_rname,' ',8);
      memcpy(fdx2_record_ptr -> fdx2_rname,PCLIB,(sizeof(PCLIB)-1));
      for (j = 0; j < 49; j++)
	{
	 if (k <= file_count)
	   {
	    memcpy(fdx2_entry_ptr -> fdx2_ename,
		   file_array[k].file_name,
		   sizeof(fdx2_entry_ptr -> fdx2_ename));
	    TO_INT(fdx2_entry_ptr -> fdx2_eblkx,
		   file_array[k].file_fdr_block);
	    k++;
	    fdx2_entry_ptr++;
	   }

	}
     if (k > (file_count+1))
       {
	chain = 13 + i;
	TO_INT(fdx2_record_ptr -> fdx2_recchn, chain);
       }
     }

   write_wang_block(drive, block, fdx2_ptr);

   return;

   }

/* this routine process the file_array and produces the
 * fdr1 blocks that describe the files in the one libaray.
 */
void process_fdr1(drive, lrecl, blksize, next_file, fdr1_block)

  char drive;
  unsigned lrecl;
  unsigned blksize;
  unsigned next_file;
  unsigned fdr1_block;

  {
   unsigned i;
   unsigned k;
   unsigned chain;
   unsigned file_block;
   unsigned zero = 0;
   unsigned key;
   unsigned char f_date[3];

   struct fdr1 *fdr1_ptr;
   struct fdr1_entry  *fdr1_entry_ptr;

   memset(fdr1buf,0,blksize);
   fdr1_ptr = (struct fdr1 *) fdr1buf;
   fdr1_ptr -> fdr_btyp = 'F';

   f_date[0] = 0x90;  /* date = 90001f     */
   f_date[1] = 0x00;
   f_date[2] = 0x1f;

   k = next_file;

   for (i = 0; i < 25; i++)
     {
      if (k > file_count)
	{
	 break;
	}
      fdr1_entry_ptr = &(fdr1_ptr -> fdr1_array[i]);
      fdr1_entry_ptr -> fdr1_format    = '1';
      fdr1_entry_ptr -> fdr1_xtntcount = 1;
      fdr1_entry_ptr -> fdr1_org       = 0x01; /* consecutive */
      fdr1_entry_ptr -> fdr1_flags     = 0x80; /* fixed       */
      key = 0xa9;                 /* first index blk in fdx1    */
      TO_INT(fdr1_entry_ptr -> fdr1_x1ptr, key);
      memcpy(fdr1_entry_ptr -> fdr1_filename,
	     file_array[k].file_name,
	     sizeof(fdr1_entry_ptr -> fdr1_filename));
      fdr1_entry_ptr -> fdr1_filesection = '1';
      memcpy(fdr1_entry_ptr -> fdr1_credate,f_date,
	     sizeof(fdr1_entry_ptr -> fdr1_credate));
      memcpy(fdr1_entry_ptr -> fdr1_moddate,f_date,
	     sizeof(fdr1_entry_ptr -> fdr1_moddate));
      memcpy(fdr1_entry_ptr -> fdr1_expdate,f_date,
	     sizeof(fdr1_entry_ptr -> fdr1_expdate));
      fdr1_entry_ptr -> fdr1_fpclass = ' ';
      memset(fdr1_entry_ptr -> fdr1_creator,'?',
	     sizeof(fdr1_entry_ptr -> fdr1_creator));
      TO_INT(fdr1_entry_ptr -> fdr1_blksize,blksize);
      TO_INT(fdr1_entry_ptr -> fdr1_secext, zero);
      file_block = file_array[k].file_start;
      TO_INT3(fdr1_entry_ptr -> fdr1_x1strt,file_block);
      file_block = file_array[k].file_end + 1;
      TO_INT3(fdr1_entry_ptr -> fdr1_x1end, file_block);
      memset(fdr1_entry_ptr -> fdr1_spare2,' ',
	    sizeof(fdr1_entry_ptr -> fdr1_spare2));
      TO_INT4(fdr1_entry_ptr -> fdr1_nrecs,
	      file_array[k].file_records);
      TO_INT(fdr1_entry_ptr -> fdr1_recsize, lrecl);
      file_block = file_array[k].file_end -
		   file_array[k].file_start;
      TO_INT3(fdr1_entry_ptr -> fdr1_eblk, file_block);
      TO_INT(fdr1_entry_ptr -> fdr1_erec,
	     file_array[k].file_block_last);
      file_array[k].file_fdr_block = fdr1_block - 1;

      k++;
    }

   write_wang_block(drive, fdr1_block, fdr1_ptr);

   return;

   }

/* this routine loads the actual PC records into the WANG blocks
 * and then write the blocks to disk
 */

unsigned process_data(record, lrecl, blksize,
		      drive, function, free_block)

  char *record;
  unsigned lrecl;
  unsigned blksize;
  char drive;
  char function;
  unsigned *free_block;

  {
    static block_i;
    unsigned packing;
    unsigned block;

    if (*free_block > 569)
      {
       printf("error - diskette out of space\n");
       return(0);
      }

    packing = blksize / lrecl;

    switch (function)
      {
       case 'O':
	 block_i = 0;
	 memset(diskbuf,0,blksize);
	 break;

       case 'W':
	 if (block_i >= packing)
	   {
	    block = *free_block;
	    write_wang_block(drive, block, diskbuf);
	    block_i = 0;
	    memset(diskbuf,0,blksize);
	    (*free_block)++;
	   }
	 memcpy(&(diskbuf[(block_i * lrecl)]),record,lrecl);
	 block_i++;
	 break;

       case 'C':
	 /* need to write last block */
	 if (block_i > 0)
	   {
	    block = *free_block;
	    write_wang_block(drive, block, diskbuf);
	   }
	 (*free_block)++;

	 break;
      }
    return(block_i);
  }

/* this routine handles the actual coping of the wang file to
 * a PC file.
 */

void wang_copy(pc_file, file_entry_ptr, drive, free_block)

  char *pc_file;
  struct f_name *file_entry_ptr;
  char drive;
  unsigned *free_block;

  {

   unsigned records;
   char record[90];
   unsigned offset;
   unsigned blksize;
   unsigned lrecl;
   unsigned packing;
   unsigned i, j, k;
   unsigned block;
   char ch;

   FILE *fopen(), *fp;

   printf("\n");
   printf("Copying PC file to %8.8s ", file_entry_ptr -> file_name);
   printf("on WANG diskette.\n");


   fp = fopen(pc_file, "r");
   if (fp == NULL)
     {
      printf("\n");
      printf("error -  %s will not open.", pc_file);
      return;
     }

   lrecl   = 80;
   blksize = 2048;

   records = 0;
   offset = 0;

   file_entry_ptr -> file_start = *free_block;
   process_data(record, lrecl, blksize, drive, 'O', free_block);

   memset(record,' ',lrecl);

   while (feof(fp) == 0)
     {
       fgets(record,(lrecl+2),fp);  /* +2 = \0 and \n    */
       if (feof(fp) == 0)
	 {
	  for (i=0; i<lrecl; i++)
	    {
	     if (isgraph(record[i]) == 0)
	       {
		record[i] = ' ';
	       }
	    }
	  process_data(record, lrecl, blksize,
		       drive, 'W', free_block);
	  records++;
	  memset(record,' ',lrecl);
	 }
     }

     fclose(fp);

     file_entry_ptr -> file_end = *free_block;
     file_entry_ptr -> file_records = records;
     file_entry_ptr -> file_block_last =
	 process_data(record, lrecl, blksize, drive, 'C', free_block);

     printf("records copied: %d\n", records);

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

/* this routine writes one "wang block" which is 2k long
 * to the floppy drive.
 */
void write_wang_block(drive, block_id, block_ptr)

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

				  /* write 1rst pc sector        */
    write_pc_block(drive, internal_block_id, ptr);

				  /* write 2nd pc sector         */
    internal_block_id++;
    ptr += pc_sector_size;
    write_pc_block(drive, internal_block_id, ptr);

				  /* write 3rd pc sector         */
    internal_block_id++;
    ptr += pc_sector_size;
    write_pc_block(drive, internal_block_id, ptr);

				  /* write 4th pc sector         */
    internal_block_id++;
    ptr += pc_sector_size;
    write_pc_block(drive, internal_block_id, ptr);

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

/* this routine writes one "pc block" which is 512 bytes long
 * to the floppy drive.
 */
void write_pc_block(drive, pc_block_id, block_ptr)

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


    status = _bios_disk( _DISK_WRITE, &di ) >> 8;

    if( status )
      {
       printf( "Error: 0x%.2x\n", status );
       exit(1);
      }

   return;
  }







