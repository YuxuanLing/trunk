#include "annexb.h"

#include <stdio.h>
#include <stdlib.h>

#include "getopt.h"                     /* included last since it redefines getopt-stuff */

#define MAX_NALUS_TO_DROP 128
unsigned nalus_to_drop [MAX_NALUS_TO_DROP];


static void help (
  const char * progname)
{
  printf ("Usage: %s [options] INFILE OUTFILE [N1 [N2 [N3 ...]]]\n"
          "  Program to drop NALUs N1...NX from H.264 INFILE and write to OUTFILE.\n"
          "  Options:\n"
          "    -h, --help              show this help message\n"
          "    -v, --verbose           verbose output\n",
          progname);
}


static void usage(
  const char * progname)
{
  fprintf(stderr,
          "Summary: %s [-help] [options] INFILE OUTFILE [N1 [N2 [N3 ...]]]\n",
          progname);
}


static int parse_options (
  const char * progname,
  int          argc,
  char *       argv[],
  int *        verbose,
  int *        num_nalus_to_drop,
  char * *     filename_in_ptr,
  char * *     filename_out_ptr)
{
  char * shortopts = "hv";
  struct option longopts[] =
  {
    /* name,        has_arg,           flag, val */         /* longind */
    { "help",       no_argument,       0,    'h' },         /*       1 */
    { "verbose",    no_argument,       0,    'v' },         /*       2 */
    /* end-of-list marker */
    { 0, 0, 0, 0 }
  };
  int opt;
  int longind = 0;


  /* Parse optional arguments */
  while ((opt = getopt_long_only(argc, argv, shortopts, longopts, &longind)) != -1)
  {
    switch (opt)
    {
    case 'h':         /* -help */
      help(progname);
      return 0;
    case 'v':         /* -verbose */
    {
      *verbose = 1;
      break;
    }
    case '?':         /* getopt_long_only noticed an error */
      usage(progname);
      return 0;
    default:         /* something unexpected has happened */
      fprintf(stderr, "%s: getopt_long_only returned an unexpected value (%d)\n",
              progname, opt);
      return 0;
    }
  }

  /* Parse mandatory arguments */
  if (argc - optind < 2)
  {
    usage (progname);
    return 0;
  }

  *filename_in_ptr = argv[optind++];
  *filename_out_ptr = argv[optind++];

  *num_nalus_to_drop = argc - optind;
  if (*num_nalus_to_drop > MAX_NALUS_TO_DROP)
  {
    fprintf (stderr,
             "Number of nalus to drop exceeds the maximum (%d > %d)\n",
             *num_nalus_to_drop, MAX_NALUS_TO_DROP);
    exit (1);
  }

  for (int i = 0; i < *num_nalus_to_drop; i++)
    nalus_to_drop[i] = (unsigned) atoi (argv[optind++]);

  return 1;
}


static int compare_uint (
  const void * a,
  const void * b)
{
  return ( *(unsigned *) a - *(unsigned *) b );
}


int main (
  int    argc,
  char * argv[])
{
  int verbose = 0;
  int num_nalus_to_drop = 0;
  char * filename_in;
  char * filename_out;
  FILE * infile;
  FILE * outfile;

  const char * progname = argv[0];
  getchar();
  printf("------starting encoder-------\n");
  if (!parse_options (progname,
                      argc,
                      argv,
                      &verbose,
                      &num_nalus_to_drop,
                      &filename_in,
                      &filename_out))
  {
    exit (1);
  }

  if (!(infile = fopen(filename_in, "rb")))
  {
    fprintf (stderr, "Error opening file %s\n", filename_in);
    exit (1);
  }

  if (!(outfile = fopen(filename_out, "wb")))
  {
    fprintf (stderr, "Error opening file %s\n", filename_out);
    exit (1);
  }

  qsort ((void *) nalus_to_drop, num_nalus_to_drop,
         sizeof (nalus_to_drop[0]), compare_uint);

  annexb_context ctx;
  annexb_init (&ctx, infile);

  int nalu_num = 0;
  int nalus_dropped = 0;
  while (extract_nalu(&ctx) != NULL)
  {
    if (nalus_dropped < num_nalus_to_drop
        && nalus_to_drop[nalus_dropped] == nalu_num)
    {
      nalus_dropped++;
      if (verbose)
      {
        printf ("Dropping NALU %d. Dropped %d, %d to go.\n",
                nalu_num, nalus_dropped, num_nalus_to_drop - nalus_dropped);
      }
    }
    else
    {
      int start_code_size = ctx.nalu_start - ctx.buf_start;
      int total_size = ctx.nalu_size + start_code_size;

      int n = fwrite (ctx.buf_start, sizeof (ctx.buf_start[0]), total_size, outfile);
      if (total_size != n)
      {
        fprintf (stderr, "Warning: Error while writing NALU to file.");
      }
    }

    /*
     * printf ("#%-4d: Got NALU of size %6u bytes pos %p %p\n",
     * nalu_num, ctx.nalu_size, ctx.buf_start, ctx.nalu_start);
     */

    nalu_num++;
  }

  printf ("Found %d NALUs, successfully dropped %d out of %d\n",
          nalu_num, nalus_dropped, num_nalus_to_drop);

  return 0;
}
