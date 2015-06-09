/* 
 * This module contains compatibility functions between windows and posix machines.
 */

#ifndef _COMPAT_
#define _COMPAT_

#include <stdlib.h>

#ifdef __cplusplus
extern "C" {
#endif

    inline void * _aligned_malloc(size_t size, size_t alignment) {
	void * ptr = NULL;
	if (0 == posix_memalign(&ptr, alignment, size)) {
	    // allocation succussful
	    return ptr;
	}
	return NULL;
    }

    inline void _aligned_free(void *memblock) {
	return free(memblock);
    }

#ifdef __cplusplus
}
#endif

#endif
