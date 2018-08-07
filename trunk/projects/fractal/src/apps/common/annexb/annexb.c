/* -*- indent-tabs-mode:nil; c-basic-offset:4 -*- */

#include "annexb.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static const size_t max_nalu_size = 1000000;
static const size_t max_start_code_size = 4;

#ifdef _WIN32
# define bool _Bool
# define true 1
# define false 0
# define __bool_true_false_are_defined 1
#else
# include <stdbool.h>
#endif


#define MIN(a, b) (((a) < (b)) ? (a) : (b))

void annexb_init (
  annexb_context * c,
  FILE *           file)
{
  c->buf_start = (unsigned char *) malloc (max_nalu_size * sizeof (unsigned char));
  if (c->buf_start == NULL)
  {
    fprintf (stderr, "Error allocating buffers for reading NALUs.\n");
    exit (1);
  }
  c->buf_size = 0;
  c->nalu_size = 0;
  c->nalu_start = c->buf_start;
  c->file = file;
}

void annexb_free (
  annexb_context * c)
{
  free (c->buf_start);
}


static size_t fill_buffer (
  unsigned char * buf,
  FILE *          file)
{
  static const size_t chunk_size = 2048;
  size_t n = fread (buf, sizeof (unsigned char), chunk_size, file);
  if (n < chunk_size)
  {
    /* decide if we have end of file or error */
    if (!feof (file))
    {
      fprintf (stderr, "Error while reading file.\n");
      exit (1);
    }
  }
  return n;
}

/* Find the length of the start code at the current position. 0 If none. */
static size_t nalu_start_code_size (
  const unsigned char * bufptr,
  size_t                size)
{
  size_t ret = 1;

  while (!*bufptr && --size)
  {
    bufptr++;
    ret++;
  }
  return *bufptr == 0x01 ? ret : 0;
}


/* Returns the number of bytes before the next NAL unit. If no NAL is found it
 * will return a value equal to size. */
static size_t search_nalu_start_code (
  const unsigned char * bufptr,
  size_t                size)
{
  size_t bytes_before_sc = 0;
  bool nalu_found = false;
  while (size && nalu_found == false)
  {
    size_t sc_size = nalu_start_code_size (bufptr, size);
    if (sc_size == 3 || sc_size == 4)
    {
      nalu_found = true;
    }
    else
    {
      bytes_before_sc++;
      bufptr++;
      size--;
    }
  }

  return bytes_before_sc;
}


/* Goes forward in file until the end of the NALU. Ensures that the whole
 * NALU is in the buffer. Returns the size of the NALU */
static size_t fill_nalu_into_buffer (
  annexb_context * c)
{
  bool next_nalu_found = false;
  unsigned char * bufptr = c->nalu_start;
  size_t nalu_offset = c->nalu_start - c->buf_start;
  c->nalu_size = 0;

  do
  {
    size_t bytes_left  = c->buf_size - c->nalu_size - nalu_offset;
    size_t searched_bytes = search_nalu_start_code (bufptr, bytes_left);
    c->nalu_size += searched_bytes;
    if (searched_bytes == bytes_left)
    {
      /* No start code was found. */
      bufptr = c->buf_start + c->buf_size;
      size_t b = fill_buffer (bufptr, c->file);
      c->buf_size += b;
      if (b == 0)
      {
        /* End of file */
        next_nalu_found = true;
      }
      else
      {
        /* Go back a few bytes in order to detect start codes that fall
         * on chunk boundaries. */
        int rollback = MIN (max_start_code_size, bufptr - c->nalu_start);
        bufptr -= rollback;
        c->nalu_size -= rollback;
      }
    }
    else
    {
      next_nalu_found = true;
    }
  } while (next_nalu_found == false);

  return c->nalu_size;
}


/* Extracts the next NALU and returns a pointer to it. Returns NULL if there
 * are no more NALUs left in file. */
unsigned char * extract_nalu (
  annexb_context * c)
{
  /* Move the remaining bytes of after the previous NALU to the beginning
   * of the buffer. */
  size_t remaining_bytes = (c->buf_size - c->nalu_size) - (c->nalu_start - c->buf_start);
  memmove(c->buf_start, c->nalu_start + c->nalu_size, remaining_bytes);
  c->buf_size = remaining_bytes;

  if (c->buf_size < max_start_code_size)
  {
    size_t n = fill_buffer (c->buf_start + c->buf_size, c->file);
    if (n == 0)
      return NULL;
    c->buf_size += n;
  }

  /* Should always be a start code on the current buffer position, if not
   * there is something wrong. */
  size_t sc_size = nalu_start_code_size (c->buf_start, c->buf_size);
  if (sc_size < 3 || sc_size > 4)
  {
    //fprintf (stderr, "ERROR: Start code size is unexpected (size = %d)\n", sc_size);
    exit (1);
  }
  c->nalu_start = c->buf_start + sc_size;
  fill_nalu_into_buffer (c);

  return c->nalu_start;
}

#if 0
int main (
  int    argc,
  char * argv[])
{
  FILE * f = fopen (argv[1], "rb");
  int i = 0;
  if (f == NULL)
  {
    fprintf (stderr, "Error opening file %s\n", argv[1]);
    exit (1);
  }

  annexb_context ctx;

  annexb_init (&ctx, f);

  while (extract_nalu(&ctx) != NULL)
    printf ("#%4d: Got NALU of size %u at pos %p\n", i++, ctx.nalu_size, ctx.nalu_start);

  return 0;
}
#endif
