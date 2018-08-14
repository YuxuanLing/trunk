#ifndef _ENC_CABAC_ENGINE_H_
#define _ENC_CABAC_ENGINE_H_

#include "taah264stdtypes.h"


/************************************************************************
* D e f i n i t i o n s
***********************************************************************
*/

// some definitions to increase the readability of the source code

#define B_BITS         10    // Number of bits to represent the whole coding interval
#define BITS_TO_LOAD   16
#define MAX_BITS       26          //(B_BITS + BITS_TO_LOAD)
#define MASK_BITS      18          //(MAX_BITS - 8)
#define ONE            0x04000000  //(1 << MAX_BITS)
#define ONE_M1         0x03FFFFFF  //(ONE - 1)
#define HALF           0x01FE      //(1 << (B_BITS-1)) - 2
#define QUARTER        0x0100      //(1 << (B_BITS-2))
#define MIN_BITS_TO_GO 0
#define B_LOAD_MASK    0xFFFF      // ((1<<BITS_TO_LOAD) - 1)



typedef struct encoding_environment EncodingEnvironment;
typedef EncodingEnvironment *EncodingEnvironmentPtr;
typedef struct bi_context_type BiContextType;
typedef BiContextType *BiContextTypePtr;



//! struct to characterize the state of the arithmetic coding engine
struct encoding_environment
{
	int cabac_encoding;                    // todo: change this to a pointer to video_par, struct video_par *p_Vid;
	unsigned int  Elow, Erange;
	unsigned int  Ebuffer;
	unsigned int  Ebits_to_go;
	unsigned int  Echunks_outstanding;
	int           Epbuf;
	uint8_t       *Ecodestrm;
	int           *Ecodestrm_len;
	int           C;
	int           E;
};

//! struct for context management
struct bi_context_type
{
	unsigned long  count;
	uint8_t state; //uint16 state;         // index into state-table CP
	unsigned char  MPS;           // Least Probable Symbol 0/1 CP  
};



#endif
