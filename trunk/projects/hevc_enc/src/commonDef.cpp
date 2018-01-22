#include "commonDef.h"


#if _WIN32
#include <sys/types.h>
#include <sys/timeb.h>
#include <io.h>
#include <fcntl.h>
#else
#include <sys/time.h>
#endif

#define HEVC_ENC_ALIGNBYTES 32

#if _WIN32
#if defined(__MINGW32__) && !defined(__MINGW64_VERSION_MAJOR)
#define _aligned_malloc __mingw_aligned_malloc
#define _aligned_free   __mingw_aligned_free
#include "malloc.h"
#endif

void *hevc_enc_malloc(size_t size)
{
	return _aligned_malloc(size, HEVC_ENC_ALIGNBYTES);
}

void hevc_enc_free(void *ptr)
{
	if (ptr) _aligned_free(ptr);
}

#else // if _WIN32
void *hevc_enc_malloc(size_t size)
{
	void *ptr;

	if (posix_memalign((void**)&ptr, HEVC_ENC_ALIGNBYTES, size) == 0)
		return ptr;
	else
		return NULL;
}

void hevc_enc_free(void *ptr)
{
	if (ptr) free(ptr);
}

#endif // if _WIN32