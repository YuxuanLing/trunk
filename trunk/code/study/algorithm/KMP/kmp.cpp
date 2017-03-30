#include "cinclude.h"

/************************************************************************/
/* ×Ö·û´®Æ¥Åä KMPËã·¨                                                                     */
/************************************************************************/
int   __cdecl   strcmp_s(
	const   char   *   src,
	const   char   *   dst, 
	int len
)
{
	int   ret = 0;

	while ((len > 0)&&(!(ret = *(unsigned   char   *)src - *(unsigned   char   *)dst)))
		++src, ++dst, len--;

	if (ret < 0)
		ret = -1;
	else   if (ret > 0)
		ret = 1;

	return(ret);
}



int calc_subPartern_val(IN char *partern, IN int len)
{
	int ret = 0, loop;
	char parternRsv[256] = { 0 };

    for (loop = 0; loop < len  - 1; loop++) {
		if (0 == strcmp_s(&partern[0], &partern[len - 1 - loop], loop + 1)) {
			ret = loop + 1;
		}
	}

	return ret;
}


void generate_partern_table(IN char *partern, IN int len, OUT int *table)
{
	int pos = 0;

	while (pos < len) {
		table[pos] = calc_subPartern_val(partern, pos + 1);
		pos++;
	}
}

void main()
{
	char *partern = "ABCDABD";
	int table[7] = { 0 };
	generate_partern_table(partern, 7, table);

	getchar();

}

