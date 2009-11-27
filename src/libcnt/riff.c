/********************************************************************************
 *                                                                              *
 * this file is part of:                                                        *
 * libeep, the project for reading and writing avr/cnt eeg and related files    *
 *                                                                              *
 ********************************************************************************
 *                                                                              *
 * LICENSE:Copyright (c) 2003-2009,                                             *
 * Advanced Neuro Technology (ANT) B.V., Enschede, The Netherlands              *
 * Max-Planck Institute for Human Cognitive & Brain Sciences, Leipzig, Germany  *
 *                                                                              *
 ********************************************************************************
 *                                                                              *
 * This library is free software; you can redistribute it and/or modify         *
 * it under the terms of the GNU Lesser General Public License as published by  *
 * the Free Software Foundation; either version 3 of the License, or            *
 * (at your option) any later version.                                          *
 *                                                                              *
 * This library is distributed WITHOUT ANY WARRANTY; even the implied warranty  *
 * of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the              *
 * GNU Lesser General Public License for more details.                          *
 *                                                                              *
 * You should have received a copy of the GNU Lesser General Public License     *
 * along with this program. If not, see <http://www.gnu.org/licenses/>          *
 *                                                                              *
 *******************************************************************************/

#include <cnt/riff.h>
#include <eep/eepraw.h>

#ifdef COMPILE_RCS
char RCS_riff_h[] = RCS_RIFF_H;
char RCS_riff_c[] = "$RCSfile: riff.c,v $ $Revision: 2415 $";
#endif

#ifndef SEEK_SET
# define SEEK_SET 0
#endif
#ifndef SEEK_CUR
# define SEEK_CUR 1
#endif
#ifndef SEEK_END
# define SEEK_END 2
#endif

#define CHUNKHEADER_SIZE 8
#define PARENTHEADER_SIZE 12


int get_id(FILE *f, fourcc_t *in)
{
  char id[4];

  fread(id, 4, 1, f);
  *in = FOURCC(id[0], id[1], id[2], id[3]);

  return ferror(f);
}

int put_id(FILE *f, fourcc_t out)
{
  char id[4];

  id[0] =  (char) (out & 0xff);
  id[1] =  (char) ((out >> 8) & 0xff);
  id[2] =  (char) ((out >> 16) & 0xff);
  id[3] =  (char) ((out >> 24) & 0xff);

  fwrite(id, 4, 1, f);

  return ferror(f);
}

int get_chunk(FILE *f, chunk_t *in)
{
  in->start = ftell(f);
  get_id(f, &(in->id));
  read_s32(f, &(in->size));

  return ferror(f);
}

int put_chunk(FILE *f, chunk_t out)
{
  put_id(f, out.id);
  write_u32(f, out.size);

  return ferror(f);
}

int riff_form_open(FILE *f, chunk_t *chunk, fourcc_t *formtype)
{
  rewind(f);

  chunk->parent = NULL;
  get_chunk(f, chunk);
  if (chunk->id == FOURCC_RIFF) {
    get_id(f, formtype);
    return RIFFERR_NONE;
  }
  else {
    return RIFFERR_NOCHUNK;
  }
}

int riff_list_open(FILE *f, chunk_t *chunk, fourcc_t listtype, chunk_t parent)
{
  fourcc_t curlisttype;
  char match = 0;
  long nextchunk = 0;
  long skipsize = 0;
  
  /* locate the start of our tree level (the parents data area) */

  fseek(f, parent.start + PARENTHEADER_SIZE, SEEK_SET);
  do {
    fseek(f, nextchunk, SEEK_CUR);
    if (get_chunk(f, chunk)) return RIFFERR_FILE;
    if (chunk->id == FOURCC_LIST) {
      get_id(f, &curlisttype);
      if (curlisttype == listtype) {
        match = 1;
      }
      else {
        skipsize += chunk->size + CHUNKHEADER_SIZE + (chunk->size & 0x01);
        nextchunk = chunk->size - 4 + (chunk->size & 0x01);
      }
    }
    else {
      skipsize += chunk->size + CHUNKHEADER_SIZE + (chunk->size & 0x01);
      nextchunk = chunk->size + (chunk->size & 0x01);
    }
  } while (!match && skipsize < parent.size - 1);
  
  if (match)
    return RIFFERR_NONE;
  else
    return RIFFERR_NOCHUNK;
  
}

int riff_open(FILE *f, chunk_t *chunk, fourcc_t id, chunk_t parent)
{
  char match = 0;
  long nextchunk = 0;
  long skipsize = 0;
  
  /* go to parent data area */
  fseek(f, parent.start + PARENTHEADER_SIZE, SEEK_SET);
  
  /* loop true the childs on this level, no recursion into tree! */
  do {
    fseek(f, nextchunk, SEEK_CUR);
    if (get_chunk(f, chunk)) return RIFFERR_FILE;
    // printf("%8lX\t%8lX\t%8lX\n", chunk->id, chunk->size, ftell(f)-8);
    if (chunk->id == id) {
      match = 1;
    }
    else {
      skipsize += chunk->size + CHUNKHEADER_SIZE + (chunk->size & 0x01);
      nextchunk = chunk->size + (chunk->size & 0x01);
    }
  } while (!match && skipsize < parent.size);

  if (match) {
    return RIFFERR_NONE;
  } else {
    return RIFFERR_NOCHUNK;
  }

}

int riff_fetch(FILE *f, chunk_t *chunk, fourcc_t *listid, 
               chunk_t parent, int child)
{
  int s, i = 0;
  long got = 0;
  
  /* locate parent data area start */
  fseek(f, parent.start + PARENTHEADER_SIZE, SEEK_SET);
  
  s = get_chunk(f, chunk);
  while (!s && i != child && got + chunk->size < parent.size)
  {
    fseek(f, chunk->size + (chunk->size & 1), SEEK_CUR);
    got += CHUNKHEADER_SIZE + chunk->size + (chunk->size & 1);
    s = get_chunk(f, chunk);
    i++;
  }
  
  if (s || got + chunk->size > parent.size) {
    return RIFFERR_NOCHUNK;
  }
  else {
    if (chunk->id == FOURCC_LIST)
      get_id(f, listid);
    return RIFFERR_NONE;
  }
}

int riff_form_new(FILE *f, chunk_t *chunk, fourcc_t formtype)
{
  rewind(f);
  
  chunk->id = FOURCC_RIFF;
  chunk->parent = NULL;
  chunk->start = 0;
  chunk->size = 4;

  if (put_chunk(f, *chunk)) return RIFFERR_FILE;
  if (put_id(f, formtype)) return RIFFERR_FILE;

  return RIFFERR_NONE;
}


int riff_list_new(FILE *f, chunk_t *chunk, fourcc_t listtype, chunk_t *parent)
{
  chunk_t *x;

  chunk->id = FOURCC_LIST;
  chunk->start = ftell(f);
  chunk->size = 4;
  chunk->parent = parent;


  if (put_chunk(f, *chunk)) return RIFFERR_FILE;
  if (put_id(f, listtype)) return RIFFERR_FILE;

  x = chunk;
  while (x->parent != NULL) {
    x = x->parent;
    x->size += PARENTHEADER_SIZE;
  }

  return RIFFERR_NONE;
}


int riff_new(FILE *f, chunk_t *chunk, fourcc_t chunktype, chunk_t *parent)
{
  chunk_t *x;

  /*fseek(f, 0, SEEK_END);*/

  chunk->id = chunktype;
  chunk->start = ftell(f);
  chunk->parent = parent;
  chunk->size = 0;

  if (put_chunk(f, *chunk)) return RIFFERR_FILE;
  x = chunk;
  while (x->parent != NULL) {
    x = x->parent;
    x->size += CHUNKHEADER_SIZE;
  }

  return ferror(f);
}

int riff_close(FILE *f, chunk_t chunk)
{
  long fillbytes;
  long start;
  chunk_t *x;
  char junk = '\0';

  /*fseek(f, 0, SEEK_END);*/
  start = ftell(f);
  fillbytes = start & 0x01;

  /* write the chunk header */
  fseek(f, chunk.start, SEEK_SET);
  if (put_chunk(f, chunk)) return RIFFERR_FILE;

  /* tell the parents about their new size */
  x = &chunk;
  while (x->parent != NULL) {
    x = x->parent;
    x->size += fillbytes + chunk.size;
    fseek(f, x->start, SEEK_SET);
    if (put_chunk(f, *x)) return RIFFERR_FILE;
  }

  /* force next start at even filepos */
  /*fseek(f, 0, SEEK_END); Next line will be equivalent in case of seek_end == true */
  fseek(f, start, SEEK_SET);
  if (fillbytes) fwrite(&junk, 1, 1, f);

  return RIFFERR_NONE;
}

int riff_write(const char *buf, size_t size, size_t num_items,
               FILE *f, chunk_t *chunk)
{
  long sizeinc = size * num_items;
  if (fwrite(buf, size, num_items, f) != num_items) return RIFFERR_FILE;
  chunk->size += sizeinc;

  return RIFFERR_NONE;
}

int riff_read(char *buf, size_t size, size_t num_items, 
                    FILE *f, chunk_t chunk)
{
  if (fread(buf, size, num_items, f) != num_items) return RIFFERR_FILE;
  
  return RIFFERR_NONE;
}

int riff_seek(FILE *f, long offset, int whence, chunk_t chunk)
{
  long effpos=0;
  
  switch (whence) {
    case SEEK_SET: effpos = chunk.start + CHUNKHEADER_SIZE + offset;
                   break;
    case SEEK_CUR: effpos = offset;
                   break;
    case SEEK_END: effpos = chunk.start + CHUNKHEADER_SIZE + chunk.size;
                   break;
  }
  if (fseek(f, effpos, (whence != SEEK_CUR) ? SEEK_SET : SEEK_CUR))
    return RIFFERR_FILE;
  else
    return RIFFERR_NONE;
}

long get_chunk_size(chunk_t chunk)
{
  return chunk.size;
}
fourcc_t get_chunk_id(chunk_t chunk)
{
  return chunk.id;
}
