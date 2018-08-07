/* -*- indent-tabs-mode:nil; c-basic-offset:4 -*- */
#include <stdlib.h>
#include <stdio.h>


int writeSyntaxElement_Level_VLC1(
  int level)
{
  int levabs = abs(level);
  int sign = (level < 0 ? 1 : 0);

  int iCodeword, iLength;
  if (levabs < 8)
  {
    iLength = levabs * 2 + sign - 1;
    iCodeword = 1;
  }
  else if (levabs < 16)   /* 8+8) */
  {
    /* escape code1 */
    /* se->len = 14 + 1 + 4; */
    iLength = 19;
    iCodeword = (1 << 4) | ((levabs - 8) << 1) | sign;
  }
  else
  {
    iLength = 28;
    int numPrefix = 15;
    int addbit, offset;
    int levabsm16 = levabs - 16;

    /* escape code2 */
    if ((levabsm16) >= 2048)
    {
      numPrefix++;
      while ((levabsm16) >= (1 << (numPrefix - 3)) - 4096)
      {
        numPrefix++;
      }
    }

    addbit   = numPrefix - 15;
    iLength += (addbit << 1);
    offset   = (2048 << addbit) - 2048;

    iCodeword = (1 << (12 + addbit)) | ((levabsm16) << 1) | sign;
  }

  printf ("code %x  len %d\n", iCodeword, iLength);

  return 0;
}





int writeSyntaxElement_Level_VLCN(
  int level,
  int vlc)
{
  int iCodeword;
  int iLength;

  int sign = (level < 0 ? 1 : 0);
  int levabs = abs(level) - 1;

  int shift = vlc - 1;
  int escape = (15 << shift);

  if (levabs < escape)
  {
    printf ("levabs < escape\n");
    int sufmask = ~((0xffffffff) << shift);
    int suffix = (levabs) & sufmask;
    int numPrefix = (levabs) >> shift;

    iLength = numPrefix + vlc + 1;
    iCodeword = (1 << (shift + 1)) | (suffix << 1) | sign;
  }
  else
  {
    int addbit, offset;
    int levabsesc = levabs - escape;
    int numPrefix = 15;

    iLength = 28;

    if ((levabsesc) >= 2048)
    {
      numPrefix++;
      while ((levabsesc) >= (1 << (numPrefix - 3)) - 4096)
      {
        numPrefix++;
      }
    }

    addbit  = numPrefix - 15;

    iLength += (addbit << 1);
    offset = (2048 << addbit) - 2048;

    iCodeword = (1 << (12 + addbit)) | ((levabsesc - offset) << 1) | sign;

  }
  printf ("code %x  len %d\n", iCodeword, iLength);

  return 0;
}


int main (
  int    argc,
  char * argv[])
{
  if (argc % 2 != 1)
  {
    printf ("usage: %s level1 vlcnum1 level2 vlcnum2 ...\n", argv[0]);
    return 1;
  }
  for (int i = 1; i < argc; i += 2)
  {
    int level = atoi (argv[i]);
    int vlcnum = atoi (argv[i + 1]);
    printf ("level %d  vlcum %d  -->  ", level, vlcnum);
    if (vlcnum == 0)
      writeSyntaxElement_Level_VLC1 (level);
    else
      writeSyntaxElement_Level_VLCN (level, vlcnum);
  }

  return 0;
}
