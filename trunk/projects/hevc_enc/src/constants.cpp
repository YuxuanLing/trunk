#include "constants.h"





/* bFilter = g_intraFilterFlags[dir] & trSize */
const UINT8 g_intraFilterFlags[NUM_INTRA_MODE] =
{
	0x38, 0x00,
	0x38, 0x30, 0x30, 0x30, 0x30, 0x30, 0x30, 0x20, 0x00, 0x20, 0x30, 0x30, 0x30, 0x30, 0x30, 0x30,
	0x38, 0x30, 0x30, 0x30, 0x30, 0x30, 0x30, 0x20, 0x00, 0x20, 0x30, 0x30, 0x30, 0x30, 0x30, 0x30,
	0x38,
};